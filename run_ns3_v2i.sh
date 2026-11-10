#!/usr/bin/env bash
# ==============================================================================
# Kịch bản tự động biên dịch và chạy mô phỏng NS-3 + V2I Socket trên máy ảo Ubuntu
# Sinh viên thực hiện:
#   1. Võ Huy Hùng            - MSSV: 106230014 - Lớp: 23KTMT1
#   2. Lê Nguyễn Trường Giang - MSSV: 106230009 - Lớp: 23KTMT1
# ==============================================================================

set -e

NS3_DIR="${1:-$HOME/ns-allinone-3.35/ns-3.35}"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "=================================================================="
echo " [NS-3 V2I] Đồng bộ mã nguồn vanet-v2i-socket.cc và kịch bản 30 xe"
echo "=================================================================="

if [ ! -d "$NS3_DIR" ]; then
  echo "Không tìm thấy thư mục NS-3 tại: $NS3_DIR"
  echo "Cách dùng: ./run_ns3_v2i.sh /duong/dan/toi/ns-3.xx"
  exit 1
fi

cp "$PROJECT_DIR/ns3_simulation/vanet-v2i-socket.cc" "$NS3_DIR/scratch/vanet-v2i-socket.cc"
cp "$PROJECT_DIR/ns3_simulation/ns3_mobility_30nodes.tcl" "$NS3_DIR/ns3_mobility_30nodes.tcl"

echo "[1/3] Khởi chạy Central Monitoring Server (UDP:6000, Web:8080) ở chế độ nền..."
python3 "$PROJECT_DIR/v2i_socket_system/v2i_central_server.py" &
SERVER_PID=$!
sleep 1

echo "[2/3] Biên dịch và chạy NS-3 (30 OBU + 4 RSU cố định, OLSR, 802.11p, V2I UDP Socket)..."
cd "$NS3_DIR"
if [ -f "./waf" ]; then
  ./waf --run "scratch/vanet-v2i-socket --scenario=2 --protocol=1 --nodes=30 --rsuNodes=4 --v2iPort=9000 --liveUdpPort=6000 --totaltime=180.0 --traceFile=ns3_mobility_30nodes.tcl"
elif [ -f "./ns3" ]; then
  ./ns3 run "scratch/vanet-v2i-socket --scenario=2 --protocol=1 --nodes=30 --rsuNodes=4 --v2iPort=9000 --liveUdpPort=6000 --totaltime=180.0 --traceFile=ns3_mobility_30nodes.tcl"
fi

echo "[3/3] Sao chép kết quả thống kê về thư mục dự án..."
cp -f "$NS3_DIR/v2i_socket_telemetry.csv" "$PROJECT_DIR/data_output/" || true
cp -f "$NS3_DIR/v2i_flowmon_stats.xml" "$PROJECT_DIR/ns3_simulation/" || true
cp -f "$NS3_DIR/v2i_netanim.xml" "$PROJECT_DIR/ns3_simulation/" || true

kill $SERVER_PID 2>/dev/null || true
echo "Hoàn tất mô phỏng NS-3 V2I Socket!"
