#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Tiến trình Trạm RSU (Road-Side Unit Edge Socket Gateway)
Lắng nghe gói tin UDP từ 30 phương tiện (OBU) trên các cổng 9001, 9002, 9003, 9004 (và cổng NS-3 9000),
kiểm tra tính toàn vẹn CRC32, phát bản tin cảnh báo ngược I2V_ALERT về Socket của xe khi có sự cố,
và chuyển tiếp (Forward) dữ liệu thời gian thực lên Central Monitoring Server (cổng 6000).
"""

import argparse
import select
import socket
import sys
import time
from typing import Dict, Any

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

from v2i_protocol import (
    RSU_STATIONS,
    MSG_OBU_BSM,
    MSG_OBU_EMERGENCY,
    MSG_RSU_I2V_ALERT,
    MSG_RSU_TMC_FORWARD,
    pack_v2i_packet,
    unpack_v2i_packet,
    estimate_80211p_link_metrics,
)


class RsuEdgeGateway:
    def __init__(self, central_host: str = "127.0.0.1", central_port: int = 6000):
        self.central_addr = (central_host, central_port)
        self.sockets = []
        self.port_to_rsu: Dict[int, Dict[str, Any]] = {}
        self.rsu_stats: Dict[str, Dict[str, Any]] = {}
        self.forward_seq = 0

        # Khởi tạo UDP Socket cho từng trạm RSU (9001..9004) + cổng mặc định NS-3 (9000)
        for rsu in RSU_STATIONS:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            sock.bind(("0.0.0.0", rsu["port"]))
            sock.setblocking(False)
            self.sockets.append(sock)
            self.port_to_rsu[rsu["port"]] = rsu
            self.rsu_stats[rsu["id"]] = {
                "rx_pkts": 0,
                "rx_bytes": 0,
                "crc_errors": 0,
                "alerts_sent": 0,
                "forwarded_pkts": 0,
            }
            print(
                f"[RSU-GATEWAY] Khởi tạo {rsu['id']} ({rsu['name']}) "
                f"tại ({rsu['x']}, {rsu['y']}) - UDP Socket 0.0.0.0:{rsu['port']}"
            )

        ns3_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        ns3_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        ns3_sock.bind(("0.0.0.0", 9000))
        ns3_sock.setblocking(False)
        self.sockets.append(ns3_sock)
        self.port_to_rsu[9000] = RSU_STATIONS[0]

        self.tmc_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def handle_packet(self, sock: socket.socket, raw_data: bytes, client_addr):
        local_port = sock.getsockname()[1]
        default_rsu = self.port_to_rsu.get(local_port, RSU_STATIONS[0])

        header_info, payload = unpack_v2i_packet(raw_data)
        if not header_info or not payload:
            return

        rsu_id = payload.get("rsu_id", default_rsu["id"])
        rsu_meta = next((r for r in RSU_STATIONS if r["id"] == rsu_id), default_rsu)
        st = self.rsu_stats[rsu_meta["id"]]
        st["rx_pkts"] += 1
        st["rx_bytes"] += len(raw_data)

        if not header_info.get("crc_valid", True):
            st["crc_errors"] += 1
            return

        vx = float(payload.get("x", 0.0))
        vy = float(payload.get("y", 0.0))
        speed = float(payload.get("speed", 0.0))
        dist_m = ((vx - rsu_meta["x"]) ** 2 + (vy - rsu_meta["y"]) ** 2) ** 0.5
        link_info = estimate_80211p_link_metrics(dist_m, has_rsu_relay=True)

        status = payload.get("status", "NORMAL")
        if speed > 17.5 and status == "NORMAL":
            status = "OVERSPEED_WARNING"
        elif speed < 0.45 and payload.get("sim_time", 0.0) > 5.0 and status == "NORMAL":
            status = "CONGESTION_STOP"

        if status != "NORMAL" or header_info["msg_type"] == MSG_OBU_EMERGENCY:
            st["alerts_sent"] += 1
            i2v_payload = {
                "rsu_id": rsu_meta["id"],
                "target_veh": payload.get("veh_id", "unknown"),
                "alert_type": status,
                "advice": "GIAM_TOC_DO_CHU_Y_GIAO_LO" if "SPEED" in status else "CHU_Y_XE_DUNG_PHIA_TRUOC",
                "rsu_x": rsu_meta["x"],
                "rsu_y": rsu_meta["y"],
                "dist_m": round(dist_m, 2),
                "ts": time.time(),
            }
            alert_pkt = pack_v2i_packet(MSG_RSU_I2V_ALERT, st["alerts_sent"], i2v_payload)
            try:
                sock.sendto(alert_pkt, client_addr)
            except OSError:
                pass

        self.forward_seq += 1
        st["forwarded_pkts"] += 1
        forward_payload = {
            "veh_id": payload.get("veh_id", "car_01"),
            "veh_type": payload.get("veh_type", "passenger"),
            "rsu_id": rsu_meta["id"],
            "rsu_name": rsu_meta["name"],
            "x": round(vx, 2),
            "y": round(vy, 2),
            "speed": round(speed, 2),
            "angle": round(float(payload.get("angle", 0.0)), 1),
            "lane": payload.get("lane", ""),
            "status": status,
            "sim_time": float(payload.get("sim_time", 0.0)),
            "dist_to_rsu_m": round(dist_m, 2),
            "rx_power_dbm": link_info["rx_power_dbm"],
            "delay_ms": payload.get("delay_ms", link_info["delay_ms"]),
            "hops": link_info["hops"],
            "obu_seq": header_info["seq_num"],
            "obu_crc": header_info["crc32_hex"],
            "src_addr": f"{client_addr[0]}:{client_addr[1]}",
            "rsu_port": local_port,
            "rsu_rx_total": st["rx_pkts"],
            "rsu_alerts_total": st["alerts_sent"],
        }
        tmc_pkt = pack_v2i_packet(MSG_RSU_TMC_FORWARD, self.forward_seq, forward_payload)
        try:
            self.tmc_sock.sendto(tmc_pkt, self.central_addr)
        except OSError:
            pass

    def run(self, duration_sec: float = 0.0):
        print(f"[RSU-GATEWAY] Đang lắng nghe gói tin V2I trên cổng 9001..9004 và chuyển tiếp về TMC Server {self.central_addr}...")
        start_t = time.time()
        try:
            while True:
                if duration_sec > 0 and (time.time() - start_t) >= duration_sec:
                    break
                readable, _, _ = select.select(self.sockets, [], [], 0.2)
                for s in readable:
                    try:
                        data, addr = s.recvfrom(4096)
                        self.handle_packet(s, data, addr)
                    except OSError:
                        pass
        except KeyboardInterrupt:
            pass
        finally:
            for s in self.sockets:
                s.close()
            self.tmc_sock.close()
            print("[RSU-GATEWAY] Đã đóng các kết nối Socket RSU.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Trạm RSU Edge Socket Gateway")
    parser.add_argument("--server-host", default="127.0.0.1", help="Địa chỉ IP của Central Server")
    parser.add_argument("--server-port", type=int, default=6000, help="Cổng UDP của Central Server")
    parser.add_argument("--duration", type=float, default=0.0, help="Thời gian chạy (giây, 0 = vô hạn)")
    args = parser.parse_args()

    gateway = RsuEdgeGateway(args.server_host, args.server_port)
    gateway.run(duration_sec=args.duration)
