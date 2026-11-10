#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Giao thức tầng Ứng dụng V2I (Vehicle-to-Infrastructure) trên nền Socket UDP/TCP
Môn học: Lập trình mạng - Khoa Điện tử Viễn thông - ĐH Bách Khoa, ĐH Đà Nẵng
Sinh viên thực hiện:
  1. Võ Huy Hùng - MSSV: 106230014 - Lớp: 23KTMT1
  2. Lê Nguyễn Trường Giang - MSSV: 106230009 - Lớp: 23KTMT1

Cấu trúc khung tin V2I nhị phân (12-byte Binary Header + Payload UTF-8 JSON):
  +---------------+---------+----------+------------+--------------------+
  | Magic (4B)    | Ver(1B) | Type(1B) | SeqNum(2B) | CRC32 Checksum(4B) |
  |   "VAN3"      |  0x01   | 0x01..04 | 0..65535   | zlib.crc32(payload)|
  +---------------+---------+----------+------------+--------------------+
"""

import json
import math
import struct
import time
import zlib
from typing import Dict, Any, Tuple, Optional

HEADER_FORMAT = "!4sBBHI"
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)  # 12 bytes
MAGIC_BYTES = b"VAN3"
PROTOCOL_VERSION = 1

# Định nghĩa các mã bản tin (Message Types)
MSG_OBU_BSM = 0x01          # Bản tin định kỳ từ Xe (OBU) -> Trạm RSU
MSG_OBU_EMERGENCY = 0x02    # Bản tin cảnh báo khẩn cấp từ Xe (OBU) -> Trạm RSU
MSG_RSU_I2V_ALERT = 0x03    # Bản tin cảnh báo hạ tầng từ RSU -> Xe (OBU)
MSG_RSU_TMC_FORWARD = 0x04  # Bản tin chuyển tiếp từ RSU -> Server Trung tâm (TMC)

MSG_TYPE_NAMES = {
    MSG_OBU_BSM: "MSG_OBU_BSM",
    MSG_OBU_EMERGENCY: "MSG_OBU_EMERGENCY",
    MSG_RSU_I2V_ALERT: "MSG_RSU_I2V_ALERT",
    MSG_RSU_TMC_FORWARD: "MSG_RSU_TMC_FORWARD",
}

# Tọa độ 4 trạm RSU cố định đặt tại các giao lộ trọng điểm trên mạng lưới đô thị (urban_road_network.net.xml)
RSU_STATIONS = [
    {
        "id": "RSU-01",
        "name": "Trạm RSU Nút giao Tây Bắc",
        "x": 520.0,
        "y": 560.0,
        "z": 6.0,
        "ip": "10.1.0.31",
        "port": 9001,
        "range_m": 400.0,
    },
    {
        "id": "RSU-02",
        "name": "Trạm RSU Giao lộ Trung tâm Hành chính",
        "x": 880.0,
        "y": 495.0,
        "z": 6.0,
        "ip": "10.1.0.32",
        "port": 9002,
        "range_m": 400.0,
    },
    {
        "id": "RSU-03",
        "name": "Trạm RSU Đại lộ Đông Tây",
        "x": 1320.0,
        "y": 610.0,
        "z": 6.0,
        "ip": "10.1.0.33",
        "port": 9003,
        "range_m": 420.0,
    },
    {
        "id": "RSU-04",
        "name": "Trạm RSU Cửa ngõ Khu Đô thị Phía Đông",
        "x": 1780.0,
        "y": 420.0,
        "z": 6.0,
        "ip": "10.1.0.34",
        "port": 9004,
        "range_m": 420.0,
    },
]


def pack_v2i_packet(msg_type: int, seq_num: int, payload_dict: Dict[str, Any]) -> bytes:
    """Đóng gói bản tin V2I gồm Header nhị phân 12 byte và Payload JSON kèm mã kiểm tra CRC32."""
    payload_bytes = json.dumps(payload_dict, ensure_ascii=False, separators=(",", ":")).encode("utf-8")
    crc = zlib.crc32(payload_bytes) & 0xFFFFFFFF
    header = struct.pack(
        HEADER_FORMAT,
        MAGIC_BYTES,
        PROTOCOL_VERSION,
        msg_type & 0xFF,
        seq_num & 0xFFFF,
        crc,
    )
    return header + payload_bytes


def unpack_v2i_packet(raw_bytes: bytes) -> Tuple[Optional[Dict[str, Any]], Optional[Dict[str, Any]]]:
    """
    Giải mã gói tin nhận từ UDP/TCP Socket.
    Hỗ trợ cả định dạng khung nhị phân chuẩn VAN3 lẫn khung chuỗi văn bản từ NS-3 C++ (`V2I_DATA|...`).
    """
    if len(raw_bytes) >= HEADER_SIZE and raw_bytes[:4] == MAGIC_BYTES:
        magic, ver, msg_type, seq_num, rx_crc = struct.unpack(HEADER_FORMAT, raw_bytes[:HEADER_SIZE])
        payload_bytes = raw_bytes[HEADER_SIZE:]
        calc_crc = zlib.crc32(payload_bytes) & 0xFFFFFFFF
        crc_valid = rx_crc == calc_crc
        try:
            payload = json.loads(payload_bytes.decode("utf-8"))
        except Exception:
            payload = {"raw": payload_bytes.decode("utf-8", errors="replace")}
        header_info = {
            "magic": magic.decode("ascii"),
            "version": ver,
            "msg_type": msg_type,
            "msg_type_name": MSG_TYPE_NAMES.get(msg_type, f"UNKNOWN({msg_type})"),
            "seq_num": seq_num,
            "crc32_hex": f"0x{rx_crc:08X}",
            "crc_valid": crc_valid,
            "header_hex": raw_bytes[:HEADER_SIZE].hex().upper(),
            "packet_size": len(raw_bytes),
        }
        return header_info, payload

    text = raw_bytes.decode("utf-8", errors="replace").strip()
    if text.startswith("V2I_DATA|"):
        parts = text.split("|")
        seq_num = int(parts[1]) if len(parts) > 1 and parts[1].isdigit() else 0
        veh_id = f"node_{parts[2]}" if len(parts) > 2 else "node_0"
        rsu_id = parts[3] if len(parts) > 3 else "RSU-01"
        x = float(parts[4]) if len(parts) > 4 else 0.0
        y = float(parts[5]) if len(parts) > 5 else 0.0
        speed = float(parts[6]) if len(parts) > 6 else 0.0
        status = parts[7] if len(parts) > 7 else "NORMAL"
        sim_t = float(parts[8]) if len(parts) > 8 else 0.0
        delay_ms = float(parts[9]) if len(parts) > 9 else 0.23
        crc = zlib.crc32(raw_bytes) & 0xFFFFFFFF
        header_info = {
            "magic": "NS3C",
            "version": 1,
            "msg_type": MSG_OBU_EMERGENCY if status != "NORMAL" else MSG_OBU_BSM,
            "msg_type_name": "NS3_V2I_EMERGENCY" if status != "NORMAL" else "NS3_V2I_BSM",
            "seq_num": seq_num,
            "crc32_hex": f"0x{crc:08X}",
            "crc_valid": True,
            "header_hex": raw_bytes[:12].hex().upper(),
            "packet_size": len(raw_bytes),
        }
        payload = {
            "veh_id": veh_id,
            "rsu_id": rsu_id,
            "x": x,
            "y": y,
            "speed": speed,
            "status": status,
            "sim_time": sim_t,
            "delay_ms": delay_ms,
            "tx_ts": time.time(),
        }
        return header_info, payload

    return None, None


def find_nearest_rsu(x: float, y: float) -> Tuple[Dict[str, Any], float]:
    """Tìm trạm RSU gần nhất theo khoảng cách Euclid 2D."""
    best_rsu = RSU_STATIONS[0]
    best_dist = float("inf")
    for rsu in RSU_STATIONS:
        d = math.hypot(x - rsu["x"], y - rsu["y"])
        if d < best_dist:
            best_dist = d
            best_rsu = rsu
    return best_rsu, best_dist


def estimate_80211p_link_metrics(dist_m: float, has_rsu_relay: bool = True) -> Dict[str, float]:
    """
    Tính toán suy hao đường truyền Two-Ray Ground / 802.11p (5.9 GHz, TxPower = 20 dBm)
    và ước lượng độ trễ E2ED, số chặng định tuyến (hops), xác suất nhận gói thành công.
    """
    dist_eff = max(1.0, dist_m)
    rx_power_dbm = (
        20.0 - 40.0 * math.log10(dist_eff / 10.0)
        if dist_eff > 50.0
        else 20.0 - 20.0 * math.log10(dist_eff)
    )
    base_delay_ms = 0.19 + (dist_eff / 380.0) * 0.075
    if not has_rsu_relay and dist_eff > 350.0:
        hops = math.ceil(dist_eff / 280.0)
        delay_ms = base_delay_ms * hops + 0.42 * (hops - 1)
        pdr_prob = max(0.65, 1.0 - 0.08 * (hops - 1))
    else:
        hops = 1 if dist_eff <= 420.0 else 2
        delay_ms = base_delay_ms * hops + (0.055 if hops > 1 else 0.0)
        pdr_prob = 0.996 if dist_eff <= 450.0 else 0.978
    return {
        "rx_power_dbm": round(rx_power_dbm, 2),
        "delay_ms": round(delay_ms, 4),
        "hops": hops,
        "pdr_prob": pdr_prob,
    }
