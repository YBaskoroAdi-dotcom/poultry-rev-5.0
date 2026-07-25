from flask import Flask, request, jsonify, Response, render_template
import pymysql
import pymysql.cursors
import pandas as pd
import io
from datetime import datetime, timedelta

app = Flask(__name__)

# =============================================================
# KONFIGURASI DATABASE ONLINE (Clever Cloud)
# =============================================================
DB_CONFIG = {
    "host": "bfirhu7hn8arbckpenlz-mysql.services.clever-cloud.com",
    "user": "uy35lfb8urlovxnk",
    "password": "PaSzilorHOPyvTt0Jw8d",
    "database": "bfirhu7hn8arbckpenlz",
    "port": 3306,
    "autocommit": True
}

def get_db():
    return pymysql.connect(**DB_CONFIG)

# =============================================================
# HALAMAN FRONTEND
# =============================================================
@app.route("/")
def index():
    return render_template("index.html")

@app.route("/dashboard")
def dashboard():
    return render_template("dashboard.html")

@app.route("/layanan")
def layanan():
    return render_template("layanan.html")

@app.route("/tentang")
def tentang():
    return render_template("tentang.html")

# =============================================================
# 1. CREATE -> ESP32 kirim data + Otomatis Batasi Max 20 Data
# =============================================================
@app.route("/api/log", methods=["POST"])
def log_data():
    data = request.get_json(force=True)

    required = ["suhu", "kelembapan", "gas", "jarak", "status_kipas", "status_servo"]
    if not all(k in data for k in required):
        return jsonify({"error": "Field tidak lengkap", "required": required}), 400

    db = get_db()
    try:
        with db.cursor() as cur:
            # DISESUAIKAN: Masukkan ke tabel tb_riwayat
            cur.execute(
                """
                INSERT INTO tb_riwayat (suhu, kelembapan, gas, jarak, status_kipas, status_servo)
                VALUES (%s, %s, %s, %s, %s, %s)
                """,
                (
                    data["suhu"],
                    data["kelembapan"],
                    data["gas"],
                    data["jarak"],
                    data["status_kipas"],
                    data["status_servo"],
                ),
            )
            
            # FITUR PEMBERSIH: Batasi maksimal 20 data terbaru di tb_riwayat
            cur.execute("""
                DELETE FROM tb_riwayat 
                WHERE id NOT IN (
                    SELECT id FROM (
                        SELECT id FROM tb_riwayat ORDER BY id DESC LIMIT 20
                    ) AS subquery
                )
            """)
            new_id = cur.lastrowid
        return jsonify({"status": "ok", "id": new_id}), 201
    finally:
        db.close()

# =============================================================
# 2. READ -> data terbaru buat kartu real-time di dashboard
# =============================================================
@app.route("/api/latest", methods=["GET"])
def get_latest():
    db = get_db()
    try:
        with db.cursor(pymysql.cursors.DictCursor) as cur:
            cur.execute("SELECT * FROM tb_riwayat ORDER BY id DESC LIMIT 1")
            row = cur.fetchone()

        if not row:
            return jsonify({"message": "Belum ada data"}), 404

        row["waktu"] = row["waktu"].strftime("%Y-%m-%d %H:%M:%S")
        return jsonify(row)
    finally:
        db.close()

# =============================================================
# 3. READ -> history buat grafik Chart.js
# =============================================================
@app.route("/api/history", methods=["GET"])
def get_history():
    limit = request.args.get("limit", default=50, type=int)

    db = get_db()
    try:
        with db.cursor(pymysql.cursors.DictCursor) as cur:
            cur.execute("SELECT * FROM tb_riwayat ORDER BY id DESC LIMIT %s", (limit,))
            rows = cur.fetchall()

        rows.reverse()
        for r in rows:
            r["waktu"] = r["waktu"].strftime("%Y-%m-%d %H:%M:%S")

        return jsonify(rows)
    finally:
        db.close()

# =============================================================
# READ -> Data Log Harian Berdasarkan Tab Day 1-5
# =============================================================
@app.route("/api/logs_harian", methods=["GET"])
def get_logs_harian():
    day_id = request.args.get("day", default="day1")
    
    hari_ini = datetime.now().date()
    if day_id == "day2":
        target_date = hari_ini - timedelta(days=1)
    elif day_id == "day3":
        target_date = hari_ini - timedelta(days=2)
    elif day_id == "day4":
        target_date = hari_ini - timedelta(days=3)
    elif day_id == "day5":
        target_date = hari_ini - timedelta(days=4)
    else:
        target_date = hari_ini

    db = get_db()
    try:
        with db.cursor(pymysql.cursors.DictCursor) as cur:
            cur.execute(
                "SELECT * FROM tb_riwayat WHERE DATE(waktu) = %s ORDER BY waktu DESC",
                (target_date,)
            )
            rows = cur.fetchall()

        for r in rows:
            r["waktu"] = r["waktu"].strftime("%Y-%m-%d %H:%M:%S")

        return jsonify(rows)
    finally:
        db.close()

# =============================================================
# 4 & 5. READ/UPDATE -> settings
# =============================================================
@app.route("/api/settings", methods=["GET"])
def get_settings():
    db = get_db()
    try:
        with db.cursor(pymysql.cursors.DictCursor) as cur:
            cur.execute("SELECT * FROM settings WHERE id = 1")
            row = cur.fetchone()
        return jsonify(row)
    finally:
        db.close()

@app.route("/api/settings", methods=["POST"])
def update_settings():
    data = request.get_json(force=True)

    allowed_fields = ["batas_suhu", "batas_jarak", "batas_gas", "mode", "kipas_manual", "servo_manual"]
    updates = {k: v for k, v in data.items() if k in allowed_fields}

    if not updates:
        return jsonify({"error": "Tidak ada field valid untuk diupdate"}), 400

    set_clause = ", ".join(f"{k} = %s" for k in updates.keys())
    values = list(updates.values())

    db = get_db()
    try:
        with db.cursor() as cur:
            cur.execute(f"UPDATE settings SET {set_clause} WHERE id = 1", values)
        return jsonify({"status": "ok", "updated": updates})
    finally:
        db.close()

# =============================================================
# 6. EXPORT -> Unduh CSV
# =============================================================
@app.route("/api/export", methods=["GET"])
def export_csv():
    db = get_db()
    try:
        df = pd.read_sql("SELECT * FROM tb_riwayat ORDER BY id ASC", db)
        buffer = io.StringIO()
        df.to_csv(buffer, index=False)
        buffer.seek(0)

        filename = f"sensor_logs_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
        return Response(
            buffer.getvalue(),
            mimetype="text/csv",
            headers={"Content-Disposition": f"attachment; filename={filename}"},
        )
    finally:
        db.close()

# =============================================================
# 7. DELETE -> Hapus Data Log
# =============================================================
@app.route("/api/logs", methods=["DELETE"])
def delete_old_logs():
    before = request.args.get("before")
    delete_all = request.args.get("all")

    db = get_db()
    try:
        with db.cursor() as cur:
            if delete_all == "true":
                cur.execute("DELETE FROM tb_riwayat")
            elif before:
                cur.execute("DELETE FROM tb_riwayat WHERE waktu < %s", (before,))
            else:
                return jsonify({"error": "Parameter 'before' atau 'all' wajib diisi"}), 400

            deleted_count = cur.rowcount
        return jsonify({"status": "ok", "deleted_rows": deleted_count})
    finally:
        db.close()

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
