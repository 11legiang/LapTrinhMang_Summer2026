@echo off
chcp 65001 >nul
echo ============================================================================
echo   HE THONG MO PHONG MANG VANET V2I QUA SOCKET (30 XE OBU - 4 TRAM RSU - SERVER)
echo   Mon hoc: Lap trinh mang - Khoa Dien tu Vien thong - DH Bach Khoa Da Nang
echo   Sinh vien thuc hien:
echo     1. Vo Huy Hung            - MSSV: 106230014 - Lop: 23KTMT1
echo     2. Le Nguyen Truong Giang - MSSV: 106230009 - Lop: 23KTMT1
echo ============================================================================
echo.

cd /d "%~dp0"

echo [1/3] Khoi dong Central Monitoring Server (UDP Port 6000 + Web Dashboard http://localhost:8080)...
start "TMC Central Server (UDP 6000 + HTTP 8080)" cmd /k "python v2i_socket_system\v2i_central_server.py"
timeout /t 2 >nul

echo [2/3] Khoi dong 4 Tram RSU Edge Socket Gateway (UDP Ports 9001, 9002, 9003, 9004, 9000)...
start "RSU Edge Socket Gateway (RSU-01..RSU-04)" cmd /k "python v2i_socket_system\v2i_rsu_node.py"
timeout /t 1 >nul

echo [3/3] Mo trinh duyet Web Dashboard va khoi chay 30 OBU Vehicle Socket Clients...
start http://localhost:8080
start "OBU Vehicle Fleet Socket Clients (30 Xe)" cmd /k "python v2i_socket_system\v2i_obu_clients.py --interval 0.08 --loop"

echo.
echo Da khoi dong thanh cong toan bo he thong V2I Socket!
echo Truy cap Dashboard truc quan tai: http://localhost:8080
pause
