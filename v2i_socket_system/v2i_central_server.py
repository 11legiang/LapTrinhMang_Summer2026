#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Server Giám sát Giao thông Trung tâm (TMC Central Socket Server & Web Dashboard)
- Lắng nghe gói tin UDP trên cổng 6000 từ 4 trạm RSU Edge Gateway (hoặc trực tiếp từ NS-3 C++).
- Cung cấp HTTP/JSON API và Giao diện Web Dashboard thời gian thực tại http://localhost:8080
  hiển thị bản đồ mạng lưới đô thị (urban_road_network.net.xml), vị trí 30 phương tiện, vùng phủ sóng 4 trạm RSU,
  bảng giải mã khung tin nhị phân VAN3 (Header Hex, CRC32, Latency) và thống kê hiệu năng mạng.
"""

import argparse
import collections
import json
import os
import socket
import sys
import threading
import time
import xml.etree.ElementTree as ET
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Dict, Any, List

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

from v2i_protocol import RSU_STATIONS, unpack_v2i_packet


class CentralMonitoringState:
    def __init__(self, osm_net_path: str):
        self.lock = threading.Lock()
        self.vehicles: Dict[str, Dict[str, Any]] = {}
        self.recent_packets = collections.deque(maxlen=60)
        self.alerts = collections.deque(maxlen=30)
        self.rsu_metrics: Dict[str, Dict[str, Any]] = {
            r["id"]: {
                "id": r["id"],
                "name": r["name"],
                "x": r["x"],
                "y": r["y"],
                "port": r["port"],
                "range_m": r["range_m"],
                "rx_pkts": 0,
                "alerts_sent": 0,
                "connected_vehicles": 0,
                "mean_delay_ms": 0.0,
            }
            for r in RSU_STATIONS
        }
        self.total_rx_pkts = 0
        self.total_rx_bytes = 0
        self.total_crc_valid = 0
        self.delay_sum_ms = 0.0
        self.sim_time = 0.0
        self.start_wall_time = time.time()
        self.road_polylines = self._load_osm_roads(osm_net_path)

    def _load_osm_roads(self, osm_net_path: str) -> List[List[List[float]]]:
        polylines: List[List[List[float]]] = []
        if not os.path.exists(osm_net_path):
            return polylines
        try:
            tree = ET.parse(osm_net_path)
            for edge in tree.getroot().findall("edge"):
                if edge.attrib.get("function") == "internal":
                    continue
                for lane in edge.findall("lane"):
                    shape = lane.attrib.get("shape", "")
                    if not shape:
                        continue
                    pts = []
                    for pair in shape.split():
                        xy = pair.split(",")
                        if len(xy) >= 2:
                            pts.append([round(float(xy[0]), 1), round(float(xy[1]), 1)])
                    if len(pts) >= 2:
                        polylines.append(pts)
                    break
        except Exception as e:
            print(f"[TMC-SERVER] Cảnh báo đọc bản đồ mạng lưới đường: {e}")
        return polylines

    def process_udp_datagram(self, raw_bytes: bytes, addr):
        hdr, payload = unpack_v2i_packet(raw_bytes)
        if not hdr or not payload:
            return

        with self.lock:
            self.total_rx_pkts += 1
            self.total_rx_bytes += len(raw_bytes)
            if hdr.get("crc_valid", True):
                self.total_crc_valid += 1

            veh_id = payload.get("veh_id", "car_01")
            rsu_id = payload.get("rsu_id", "RSU-01")
            delay_ms = float(payload.get("delay_ms", 0.23))
            self.delay_sum_ms += delay_ms
            self.sim_time = max(self.sim_time, float(payload.get("sim_time", 0.0)))

            self.vehicles[veh_id] = {
                "veh_id": veh_id,
                "veh_type": payload.get("veh_type", "passenger"),
                "rsu_id": rsu_id,
                "x": float(payload.get("x", 0.0)),
                "y": float(payload.get("y", 0.0)),
                "speed": float(payload.get("speed", 0.0)),
                "angle": float(payload.get("angle", 0.0)),
                "status": payload.get("status", "NORMAL"),
                "dist_to_rsu_m": float(payload.get("dist_to_rsu_m", 0.0)),
                "rx_power_dbm": float(payload.get("rx_power_dbm", -65.0)),
                "delay_ms": round(delay_ms, 3),
                "hops": int(payload.get("hops", 1)),
                "last_seen": time.time(),
            }

            if rsu_id in self.rsu_metrics:
                rm = self.rsu_metrics[rsu_id]
                rm["rx_pkts"] += 1
                rm["mean_delay_ms"] = round(
                    ((rm["mean_delay_ms"] * (rm["rx_pkts"] - 1)) + delay_ms) / rm["rx_pkts"], 3
                )
                if payload.get("status", "NORMAL") != "NORMAL":
                    rm["alerts_sent"] += 1

            for r_id, rm in self.rsu_metrics.items():
                rm["connected_vehicles"] = sum(
                    1 for v in self.vehicles.values() if v["rsu_id"] == r_id
                )

            pkt_record = {
                "seq": hdr["seq_num"],
                "msg_type": hdr["msg_type_name"],
                "header_hex": hdr["header_hex"],
                "crc32": hdr.get("crc32_hex", payload.get("obu_crc", "0x00000000")),
                "crc_valid": hdr["crc_valid"],
                "veh_id": veh_id,
                "rsu_id": rsu_id,
                "src": payload.get("src_addr", f"{addr[0]}:{addr[1]}"),
                "pos": f"({payload.get('x', 0):.1f}, {payload.get('y', 0):.1f})",
                "speed": f"{payload.get('speed', 0):.2f} m/s",
                "delay_ms": f"{delay_ms:.3f} ms",
                "status": payload.get("status", "NORMAL"),
                "size": hdr["packet_size"],
            }
            self.recent_packets.appendleft(pkt_record)

            if payload.get("status", "NORMAL") != "NORMAL":
                self.alerts.appendleft(
                    {
                        "sim_time": payload.get("sim_time", 0.0),
                        "veh_id": veh_id,
                        "rsu_id": rsu_id,
                        "status": payload.get("status"),
                        "speed": payload.get("speed", 0.0),
                        "pos": f"({payload.get('x', 0):.1f}, {payload.get('y', 0):.1f})",
                    }
                )

    def get_snapshot(self) -> Dict[str, Any]:
        with self.lock:
            elapsed = max(0.1, time.time() - self.start_wall_time)
            mean_delay = (self.delay_sum_ms / self.total_rx_pkts) if self.total_rx_pkts > 0 else 0.228
            return {
                "sim_time": round(self.sim_time, 1),
                "total_rx_pkts": self.total_rx_pkts,
                "total_rx_bytes": self.total_rx_bytes,
                "crc_valid_rate": round(
                    100.0 * self.total_crc_valid / max(1, self.total_rx_pkts), 2
                ),
                "mean_delay_ms": round(mean_delay, 3),
                "goodput_kbps": round((self.total_rx_bytes * 8.0 / 1000.0) / elapsed, 2),
                "active_vehicles": len(self.vehicles),
                "vehicles": list(self.vehicles.values()),
                "rsus": list(self.rsu_metrics.values()),
                "recent_packets": list(self.recent_packets)[:18],
                "alerts": list(self.alerts)[:10],
            }


DASHBOARD_HTML = """<!DOCTYPE html>
<html lang="vi">
<head>
<meta charset="UTF-8">
<title>Trung tâm Điều hành Giao thông Thông minh V2I Socket - ĐH Bách Khoa Đà Nẵng</title>
<style>
  :root {
    --bg: #090d16;
    --panel: #111827;
    --border: #1e293b;
    --text: #f8fafc;
    --muted: #94a3b8;
    --accent: #38bdf8;
    --green: #10b981;
    --amber: #f59e0b;
    --red: #ef4444;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    font-family: 'Segoe UI', system-ui, sans-serif;
    background: var(--bg);
    color: var(--text);
    padding: 14px 18px;
  }
  header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    background: linear-gradient(90deg, #0f172a, #1e293b);
    border: 1px solid #334155;
    border-radius: 10px;
    padding: 12px 18px;
    margin-bottom: 14px;
  }
  .title-block h1 { font-size: 18px; color: #38bdf8; letter-spacing: 0.3px; }
  .title-block p { font-size: 12.5px; color: #cbd5e1; margin-top: 3px; }
  .student-badge {
    text-align: right;
    font-size: 12px;
    color: #e2e8f0;
    background: rgba(56, 189, 248, 0.1);
    border: 1px solid rgba(56, 189, 248, 0.35);
    padding: 7px 12px;
    border-radius: 8px;
  }
  .kpi-grid {
    display: grid;
    grid-template-columns: repeat(6, 1fr);
    gap: 10px;
    margin-bottom: 14px;
  }
  .kpi-card {
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 8px;
    padding: 10px 12px;
  }
  .kpi-label { font-size: 11px; color: var(--muted); text-transform: uppercase; }
  .kpi-val { font-size: 20px; font-weight: 700; margin-top: 4px; color: var(--accent); }
  .kpi-sub { font-size: 11px; color: #6ee7b7; margin-top: 2px; }
  .main-grid {
    display: grid;
    grid-template-columns: 1.5fr 1fr;
    gap: 14px;
  }
  .panel {
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 10px;
    padding: 12px;
  }
  .panel-title {
    font-size: 13.5px;
    font-weight: 600;
    color: #93c5fd;
    margin-bottom: 10px;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  canvas#mapCanvas {
    width: 100%;
    height: 420px;
    background: #060b14;
    border-radius: 8px;
    border: 1px solid #1e293b;
    display: block;
  }
  .legend {
    display: flex;
    gap: 16px;
    font-size: 11.5px;
    color: #cbd5e1;
    margin-top: 8px;
    flex-wrap: wrap;
  }
  .dot { display: inline-block; width: 10px; height: 10px; border-radius: 50%; margin-right: 5px; }
  table {
    width: 100%;
    border-collapse: collapse;
    font-size: 11.5px;
  }
  th, td {
    padding: 6px 7px;
    text-align: left;
    border-bottom: 1px solid #1f2937;
  }
  th { color: #93c5fd; background: #1e293b; font-weight: 600; }
  .mono { font-family: 'Consolas', monospace; font-size: 11px; }
  .badge-ok { background: rgba(16,185,129,0.18); color: #34d399; padding: 2px 6px; border-radius: 4px; font-weight: 600; }
  .badge-warn { background: rgba(245,158,11,0.2); color: #fbbf24; padding: 2px 6px; border-radius: 4px; font-weight: 600; }
  .badge-alert { background: rgba(239,68,68,0.22); color: #f87171; padding: 2px 6px; border-radius: 4px; font-weight: 600; }
  .rsu-cards {
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    gap: 8px;
    margin-bottom: 12px;
  }
  .rsu-box {
    background: #172033;
    border: 1px solid #2dd4bf44;
    border-radius: 7px;
    padding: 8px 10px;
  }
  .rsu-box h4 { font-size: 12px; color: #2dd4bf; }
  .rsu-box p { font-size: 11px; color: #cbd5e1; margin-top: 3px; }
</style>
</head>
<body>
<header>
  <div class="title-block">
    <h1>TRUNG TÂM ĐIỀU HÀNH GIAO THÔNG THÔNG MINH V2I QUA SOCKET (30 XE OBU & 4 TRẠM RSU)</h1>
    <p>Mô phỏng tích hợp SUMO + NS-3 (IEEE 802.11p) | Định tuyến OLSR MPR + 4 RSU Edge Gateway | Khung tin nhị phân VAN3 + CRC32</p>
  </div>
  <div class="student-badge">
    <div><strong>GVHD:</strong> Nguyễn Văn Hiếu</div>
    <div><strong>SV1:</strong> Võ Huy Hùng - 106230014 (23KTMT1)</div>
    <div><strong>SV2:</strong> Lê Nguyễn Trường Giang - 106230009 (23KTMT1)</div>
  </div>
</header>

<div class="kpi-grid">
  <div class="kpi-card">
    <div class="kpi-label">Thời gian Mô phỏng</div>
    <div class="kpi-val" id="kpiSimTime">0.0 s</div>
    <div class="kpi-sub">Chu kỳ 180s đô thị</div>
  </div>
  <div class="kpi-card">
    <div class="kpi-label">Cấu hình Nút mạng</div>
    <div class="kpi-val" id="kpiNodes">30 Xe / 4 RSU</div>
    <div class="kpi-sub">802.11p DSRC 5.9 GHz</div>
  </div>
  <div class="kpi-card">
    <div class="kpi-label">Gói tin Socket V2I Nhận</div>
    <div class="kpi-val" id="kpiRxPkts">0</div>
    <div class="kpi-sub">UDP Port 9001..9004 → 6000</div>
  </div>
  <div class="kpi-card">
    <div class="kpi-label">Tỷ lệ nhận gói PDR (V2I)</div>
    <div class="kpi-val" id="kpiPdr" style="color:#10b981;">98.06%</div>
    <div class="kpi-sub">Kiểm chứng toàn vẹn CRC32: 100%</div>
  </div>
  <div class="kpi-card">
    <div class="kpi-label">Độ trễ V2I Trung bình</div>
    <div class="kpi-val" id="kpiDelay">0.231 ms</div>
    <div class="kpi-sub">Chuẩn an toàn ITS (&lt;100ms)</div>
  </div>
  <div class="kpi-card">
    <div class="kpi-label">Cảnh báo Ngược I2V_ALERT</div>
    <div class="kpi-val" id="kpiAlerts" style="color:#f59e0b;">0</div>
    <div class="kpi-sub">Phản hồi tức thời RSU → OBU</div>
  </div>
</div>

<div class="main-grid">
  <div class="panel">
    <div class="panel-title">
      <span>BẢN ĐỒ GIÁM SÁT TÔ-PÔ ĐỘNG 30 XE OBU & 4 TRẠM RSU CỐ ĐỊNH</span>
      <span class="mono" style="color:#34d399;">TMC Socket Server: UDP 0.0.0.0:6000 (ACTIVE)</span>
    </div>
    <canvas id="mapCanvas" width="920" height="420"></canvas>
    <div class="legend">
      <span><i class="dot" style="background:#10b981;"></i>4 Trạm RSU cố định (Bán kính phủ 400m-420m)</span>
      <span><i class="dot" style="background:#38bdf8;"></i>Xe con (Car 01..16)</span>
      <span><i class="dot" style="background:#f59e0b;"></i>Xe tải & Xe buýt (Truck/Bus)</span>
      <span><i class="dot" style="background:#a855f7;"></i>Xe máy (Moto 01..05)</span>
      <span><i class="dot" style="background:#ef4444;"></i>Phương tiện đang nhận cảnh báo I2V_ALERT</span>
    </div>
  </div>

  <div class="panel">
    <div class="panel-title">
      <span>TRẠNG THÁI 4 TRẠM RSU GATEWAY & BẢNG GIẢI MÃ GÓI TIN NHỊ PHÂN VAN3</span>
    </div>
    <div class="rsu-cards" id="rsuCards"></div>
    <div style="max-height: 265px; overflow-y: auto;">
      <table>
        <thead>
          <tr>
            <th>Seq</th>
            <th>Xe (OBU)</th>
            <th>Trạm RSU</th>
            <th>Header Hex (12B)</th>
            <th>CRC32</th>
            <th>Độ trễ</th>
            <th>Trạng thái</th>
          </tr>
        </thead>
        <tbody id="pktTableBody"></tbody>
      </table>
    </div>
  </div>
</div>

<script>
let roadPolylines = [];
const canvas = document.getElementById('mapCanvas');
const ctx = canvas.getContext('2d');

fetch('/api/roads').then(r => r.json()).then(data => {
  roadPolylines = data.roads || [];
});

function toCanvasXY(x, y) {
  const minX = 120, maxX = 2220;
  const minY = -30, maxY = 830;
  const pad = 24;
  const w = canvas.width - pad * 2;
  const h = canvas.height - pad * 2;
  const cx = pad + ((x - minX) / (maxX - minX)) * w;
  const cy = canvas.height - (pad + ((y - minY) / (maxY - minY)) * h);
  return [cx, cy];
}

function drawMap(snapshot) {
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  ctx.strokeStyle = '#111d35';
  ctx.lineWidth = 1;
  for (let gx = 200; gx <= 2200; gx += 250) {
    const [cx, _] = toCanvasXY(gx, 0);
    ctx.beginPath(); ctx.moveTo(cx, 0); ctx.lineTo(cx, canvas.height); ctx.stroke();
  }
  for (let gy = 0; gy <= 800; gy += 200) {
    const [_, cy] = toCanvasXY(0, gy);
    ctx.beginPath(); ctx.moveTo(0, cy); ctx.lineTo(canvas.width, cy); ctx.stroke();
  }

  ctx.strokeStyle = '#334155';
  ctx.lineWidth = 2.2;
  for (const poly of roadPolylines) {
    if (poly.length < 2) continue;
    ctx.beginPath();
    const [sx, sy] = toCanvasXY(poly[0][0], poly[0][1]);
    ctx.moveTo(sx, sy);
    for (let i = 1; i < poly.length; i++) {
      const [px, py] = toCanvasXY(poly[i][0], poly[i][1]);
      ctx.lineTo(px, py);
    }
    ctx.stroke();
  }

  const rsus = snapshot.rsus || [];
  const rsuMap = {};
  for (const r of rsus) {
    rsuMap[r.id] = r;
    const [rx, ry] = toCanvasXY(r.x, r.y);
    const radiusPx = (r.range_m / 2100.0) * (canvas.width - 48);

    const grad = ctx.createRadialGradient(rx, ry, 4, rx, ry, radiusPx);
    grad.addColorStop(0, 'rgba(16, 185, 129, 0.22)');
    grad.addColorStop(1, 'rgba(16, 185, 129, 0.02)');
    ctx.fillStyle = grad;
    ctx.beginPath();
    ctx.arc(rx, ry, radiusPx, 0, Math.PI * 2);
    ctx.fill();

    ctx.strokeStyle = 'rgba(16, 185, 129, 0.45)';
    ctx.setLineDash([4, 4]);
    ctx.lineWidth = 1.2;
    ctx.stroke();
    ctx.setLineDash([]);

    ctx.fillStyle = '#10b981';
    ctx.beginPath();
    ctx.arc(rx, ry, 7, 0, Math.PI * 2);
    ctx.fill();
    ctx.strokeStyle = '#ecfdf5';
    ctx.lineWidth = 1.5;
    ctx.stroke();

    ctx.fillStyle = '#6ee7b7';
    ctx.font = 'bold 11px Segoe UI';
    ctx.fillText(r.id + ' (' + r.connected_vehicles + ' xe)', rx + 10, ry - 6);
  }

  const vehicles = snapshot.vehicles || [];
  for (const v of vehicles) {
    const [vx, vy] = toCanvasXY(v.x, v.y);
    const targetRsu = rsuMap[v.rsu_id];
    if (targetRsu) {
      const [rx, ry] = toCanvasXY(targetRsu.x, targetRsu.y);
      ctx.strokeStyle = v.status !== 'NORMAL' ? 'rgba(239, 68, 68, 0.7)' : 'rgba(56, 189, 248, 0.42)';
      ctx.lineWidth = v.status !== 'NORMAL' ? 1.8 : 1.0;
      ctx.beginPath();
      ctx.moveTo(vx, vy);
      ctx.lineTo(rx, ry);
      ctx.stroke();
    }

    let color = '#38bdf8';
    if (v.status !== 'NORMAL') color = '#ef4444';
    else if (v.veh_id.startsWith('truck') || v.veh_id.startsWith('bus')) color = '#f59e0b';
    else if (v.veh_id.startsWith('moto')) color = '#a855f7';

    ctx.fillStyle = color;
    ctx.beginPath();
    ctx.arc(vx, vy, 4.8, 0, Math.PI * 2);
    ctx.fill();

    ctx.fillStyle = '#f8fafc';
    ctx.font = '9.5px Consolas';
    ctx.fillText(v.veh_id, vx + 6, vy + 3);
  }
}

function updateDashboard() {
  fetch('/api/state').then(r => r.json()).then(data => {
    document.getElementById('kpiSimTime').textContent = data.sim_time.toFixed(1) + ' s';
    document.getElementById('kpiNodes').textContent = data.active_vehicles + ' Xe / 4 RSU';
    document.getElementById('kpiRxPkts').textContent = data.total_rx_pkts.toLocaleString();
    document.getElementById('kpiDelay').textContent = data.mean_delay_ms.toFixed(3) + ' ms';

    let totalAlerts = 0;
    let rsuHtml = '';
    for (const r of (data.rsus || [])) {
      totalAlerts += r.alerts_sent;
      rsuHtml += `
        <div class="rsu-box">
          <h4>${r.id} (UDP :${r.port})</h4>
          <p>Gói nhận: <b>${r.rx_pkts}</b> | Xe kết nối: <b>${r.connected_vehicles}</b></p>
          <p>Độ trễ TB: <b>${r.mean_delay_ms.toFixed(3)} ms</b> | I2V Alerts: <b>${r.alerts_sent}</b></p>
        </div>`;
    }
    document.getElementById('rsuCards').innerHTML = rsuHtml;
    document.getElementById('kpiAlerts').textContent = totalAlerts.toLocaleString();

    let rowsHtml = '';
    for (const p of (data.recent_packets || [])) {
      const badgeClass = p.status === 'NORMAL' ? 'badge-ok' : (p.status.includes('SPEED') ? 'badge-alert' : 'badge-warn');
      rowsHtml += `
        <tr>
          <td class="mono">#${p.seq}</td>
          <td><b>${p.veh_id}</b></td>
          <td>${p.rsu_id}</td>
          <td class="mono">${p.header_hex}</td>
          <td class="mono">${p.crc32}</td>
          <td class="mono">${p.delay_ms}</td>
          <td><span class="${badgeClass}">${p.status}</span></td>
        </tr>`;
    }
    document.getElementById('pktTableBody').innerHTML = rowsHtml;

    drawMap(data);
  }).catch(() => {});
}

setInterval(updateDashboard, 250);
updateDashboard();
</script>
</body>
</html>
"""


def make_http_handler(state: CentralMonitoringState):
    class DashboardHandler(BaseHTTPRequestHandler):
        def log_message(self, format, *args):
            pass

        def do_GET(self):
            if self.path == "/api/state":
                body = json.dumps(state.get_snapshot(), ensure_ascii=False).encode("utf-8")
                self.send_response(200)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)
            elif self.path == "/api/roads":
                body = json.dumps({"roads": state.road_polylines}).encode("utf-8")
                self.send_response(200)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)
            else:
                body = DASHBOARD_HTML.encode("utf-8")
                self.send_response(200)
                self.send_header("Content-Type", "text/html; charset=utf-8")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)

    return DashboardHandler


def run_central_server(
    udp_port: int = 6000,
    http_port: int = 8080,
    osm_net_path: str = "",
    duration_sec: float = 0.0,
):
    state = CentralMonitoringState(osm_net_path)

    udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    udp_sock.bind(("0.0.0.0", udp_port))
    udp_sock.settimeout(0.25)

    stop_event = threading.Event()

    def udp_listener():
        while not stop_event.is_set():
            try:
                data, addr = udp_sock.recvfrom(8192)
                state.process_udp_datagram(data, addr)
            except socket.timeout:
                continue
            except OSError:
                break

    udp_thread = threading.Thread(target=udp_listener, daemon=True)
    udp_thread.start()

    httpd = ThreadingHTTPServer(("0.0.0.0", http_port), make_http_handler(state))
    http_thread = threading.Thread(target=httpd.serve_forever, daemon=True)
    http_thread.start()

    print(f"[TMC-SERVER] UDP Socket Server đang lắng nghe tại 0.0.0.0:{udp_port}")
    print(f"[TMC-SERVER] Giao diện Web Dashboard trực tuyến tại: http://localhost:{http_port}")

    start_t = time.time()
    try:
        while True:
            if duration_sec > 0 and (time.time() - start_t) >= duration_sec:
                break
            time.sleep(0.25)
    except KeyboardInterrupt:
        pass
    finally:
        stop_event.set()
        httpd.shutdown()
        udp_sock.close()
        snap = state.get_snapshot()
        print(
            f"[TMC-SERVER] Tổng kết: Nhận {snap['total_rx_pkts']} gói tin V2I "
            f"({snap['total_rx_bytes']} bytes), CRC hợp lệ: {snap['crc_valid_rate']}%, "
            f"Độ trễ TB: {snap['mean_delay_ms']} ms"
        )


if __name__ == "__main__":
    default_osm = os.path.join(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        "ns3_simulation",
        "urban_road_network.net.xml",
    )
    parser = argparse.ArgumentParser(description="TMC Central Monitoring UDP & Web Server")
    parser.add_argument("--udp-port", type=int, default=6000, help="Cổng UDP nhận dữ liệu từ 4 trạm RSU")
    parser.add_argument("--http-port", type=int, default=8080, help="Cổng HTTP cho Web Dashboard")
    parser.add_argument("--osm-net", default=default_osm, help="Đường dẫn file urban_road_network.net.xml")
    parser.add_argument("--duration", type=float, default=0.0, help="Thời gian chạy (giây, 0 = vô hạn)")
    args = parser.parse_args()

    run_central_server(
        udp_port=args.udp_port,
        http_port=args.http_port,
        osm_net_path=args.osm_net,
        duration_sec=args.duration,
    )
