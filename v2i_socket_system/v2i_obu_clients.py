#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Tiến trình Đội xe OBU (On-Board Unit Multi-Socket Clients)
Đọc dữ liệu di chuyển từ kịch bản SUMO (`ns3_simulation/urban_vehicles_trace.xml`),
khởi tạo 30 UDP Socket riêng biệt đại diện cho 30 phương tiện (car, truck, bus, motorcycle),
đóng gói khung tin nhị phân VAN3 gửi đến 4 Trạm RSU (cổng 9001..9004) và nhận bản tin cảnh báo I2V_ALERT.
"""

import argparse
import csv
import os
import select
import socket
import sys
import time
import xml.etree.ElementTree as ET
from typing import Dict

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

from v2i_protocol import (
    MSG_OBU_BSM,
    MSG_OBU_EMERGENCY,
    find_nearest_rsu,
    estimate_80211p_link_metrics,
    pack_v2i_packet,
    unpack_v2i_packet,
)


class ObuFleetSocketSimulator:
    def __init__(
        self,
        trace_xml_path: str,
        rsu_host: str = "127.0.0.1",
        csv_output_path: str = "",
    ):
        self.trace_xml_path = trace_xml_path
        self.rsu_host = rsu_host
        if not csv_output_path:
            root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
            csv_output_path = os.path.join(root_dir, "data_output", "v2i_socket_telemetry.csv")
        self.csv_output_path = csv_output_path
        os.makedirs(os.path.dirname(self.csv_output_path), exist_ok=True)
        self.veh_sockets: Dict[str, socket.socket] = {}
        self.seq_counter = 0
        self.tx_count = 0
        self.i2v_rx_count = 0

    def get_or_create_socket(self, veh_id: str) -> socket.socket:
        if veh_id not in self.veh_sockets:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.bind(("127.0.0.1", 0))
            sock.setblocking(False)
            self.veh_sockets[veh_id] = sock
        return self.veh_sockets[veh_id]

    def poll_i2v_alerts(self, sim_time: float = 0.0):
        if not self.veh_sockets:
            return
        sock_list = list(self.veh_sockets.values())
        readable, _, _ = select.select(sock_list, [], [], 0.002)
        for s in readable:
            try:
                raw, addr = s.recvfrom(2048)
                hdr, payload = unpack_v2i_packet(raw)
                if hdr and payload:
                    self.i2v_rx_count += 1
            except OSError:
                pass

    def run_trace(self, step_interval_sec: float = 0.05, max_steps: int = 0, loop: bool = False):
        if not os.path.exists(self.trace_xml_path):
            raise FileNotFoundError(f"Không tìm thấy file trace SUMO: {self.trace_xml_path}")

        tree = ET.parse(self.trace_xml_path)
        timesteps = tree.getroot().findall("timestep")
        print(
            f"[OBU-CLIENTS] Đã nạp {len(timesteps)} bước thời gian (30 phương tiện OBU) từ {os.path.basename(self.trace_xml_path)}"
        )

        with open(self.csv_output_path, "w", newline="", encoding="utf-8") as f_csv:
            writer = csv.writer(f_csv)
            writer.writerow(
                [
                    "SimTime",
                    "Seq",
                    "VehicleId",
                    "VehicleType",
                    "TargetRsuId",
                    "RsuPort",
                    "PosX",
                    "PosY",
                    "Speed",
                    "DistanceToRsuM",
                    "RxPowerDbm",
                    "Hops",
                    "DelayMs",
                    "AlertStatus",
                    "Crc32Hex",
                    "PacketBytes",
                ]
            )

            while True:
                step_count = 0
                for ts_elem in timesteps:
                    sim_time = float(ts_elem.attrib.get("time", "0.0"))
                    vehicles = ts_elem.findall("vehicle")

                    for v in vehicles:
                        veh_id = v.attrib["id"]
                        veh_type = v.attrib.get("type", "passenger")
                        x = float(v.attrib["x"])
                        y = float(v.attrib["y"])
                        speed = float(v.attrib.get("speed", "0.0"))
                        angle = float(v.attrib.get("angle", "0.0"))
                        lane = v.attrib.get("lane", "")

                        nearest_rsu, dist_m = find_nearest_rsu(x, y)
                        link_metrics = estimate_80211p_link_metrics(dist_m, has_rsu_relay=True)

                        status = "NORMAL"
                        msg_type = MSG_OBU_BSM
                        if speed > 17.5:
                            status = "OVERSPEED_WARNING"
                            msg_type = MSG_OBU_EMERGENCY
                        elif sim_time > 5.0 and speed < 0.45:
                            status = "CONGESTION_STOP"
                            msg_type = MSG_OBU_EMERGENCY

                        self.seq_counter += 1
                        payload = {
                            "veh_id": veh_id,
                            "veh_type": veh_type,
                            "rsu_id": nearest_rsu["id"],
                            "x": round(x, 2),
                            "y": round(y, 2),
                            "speed": round(speed, 2),
                            "angle": round(angle, 1),
                            "lane": lane,
                            "status": status,
                            "sim_time": sim_time,
                            "delay_ms": link_metrics["delay_ms"],
                            "tx_ts": time.time(),
                        }
                        pkt_bytes = pack_v2i_packet(msg_type, self.seq_counter, payload)
                        hdr_info, _ = unpack_v2i_packet(pkt_bytes)

                        sock = self.get_or_create_socket(veh_id)
                        try:
                            sock.sendto(pkt_bytes, (self.rsu_host, nearest_rsu["port"]))
                            self.tx_count += 1
                        except OSError:
                            pass

                        writer.writerow(
                            [
                                sim_time,
                                self.seq_counter,
                                veh_id,
                                veh_type,
                                nearest_rsu["id"],
                                nearest_rsu["port"],
                                round(x, 2),
                                round(y, 2),
                                round(speed, 2),
                                round(dist_m, 2),
                                link_metrics["rx_power_dbm"],
                                link_metrics["hops"],
                                link_metrics["delay_ms"],
                                status,
                                hdr_info["crc32_hex"] if hdr_info else "0x00000000",
                                len(pkt_bytes),
                            ]
                        )

                    self.poll_i2v_alerts(sim_time=sim_time)
                    step_count += 1
                    if max_steps > 0 and step_count >= max_steps:
                        break
                    if step_interval_sec > 0:
                        time.sleep(step_interval_sec)

                if not loop:
                    break

        for s in self.veh_sockets.values():
            s.close()
        print(
            f"[OBU-CLIENTS] Hoàn tất phát {self.tx_count} gói tin V2I từ {len(self.veh_sockets)} xe "
            f"(Nhận {self.i2v_rx_count} bản tin phản hồi I2V_ALERT). Log lưu tại: {self.csv_output_path}"
        )


if __name__ == "__main__":
    default_trace = os.path.join(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        "ns3_simulation",
        "urban_vehicles_trace.xml",
    )
    parser = argparse.ArgumentParser(description="Bộ phát Socket OBU cho đội xe 30 nút VANET")
    parser.add_argument("--trace", default=default_trace, help="Đường dẫn file urban_vehicles_trace.xml")
    parser.add_argument("--rsu-host", default="127.0.0.1", help="IP của RSU Gateway")
    parser.add_argument("--interval", type=float, default=0.04, help="Chu kỳ mỗi bước mô phỏng (s)")
    parser.add_argument("--max-steps", type=int, default=0, help="Giới hạn số bước (0 = toàn bộ 180s)")
    parser.add_argument("--loop", action="store_true", help="Lặp lại liên tục phục vụ demo trực tiếp")
    args = parser.parse_args()

    sim = ObuFleetSocketSimulator(args.trace, rsu_host=args.rsu_host)
    sim.run_trace(step_interval_sec=args.interval, max_steps=args.max_steps, loop=args.loop)
