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
    "connect_timeout": 10,
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
    try:
        data = request.get_json(force=True, silent=True)
        if not data:
            return jsonify({"error": "Format JSON tidak valid atau data kosong"}), 400

        required = ["suhu", "kelembapan", "gas", "jarak", "status_kipas", "status_servo"]
        missing = [k for k in required if k not in data]
        if missing:
            return jsonify({"error": "Field kurang/tidak sesuai", "missing_fields": missing}), 400

        db = get_db()
        try:
            with db.cursor() as cur:
                # MENGGUNAKAN sensor_logs
                cur.execute(
                    """
                    INSERT INTO sensor_logs (suhu, kelembapan, gas, jarak, status_kipas, status_servo)
                    VALUES (%s, %s, %s, %s, %s, %s)
                    """,
                    (
                        data["suhu"],
                        data["kelembapan"],
                        data["gas"],
                        data["jarak"],
                        str(data["status_kipas"]),
                        data["status_servo"],
                    ),
                )
                
                # Batasi maksimal 20 data terbaru
                cur.execute("""
                    DELETE FROM sensor_logs 
                    WHERE id NOT IN (
                        SELECT id FROM (
                            SELECT id FROM sensor_logs ORDER BY id DESC LIMIT 20
                        ) AS subquery
                    )
                """)
                new_id = cur.lastrowid
            return jsonify({"status": "ok", "id": new_id}), 201
        finally:
            db.close()
    except Exception as e:
        return jsonify({"error": "Gagal simpan log ke Database", "details": str(e)}), 500

# =============================================================
# 2. READ -> data terbaru buat kartu real-time di dashboard
# =============================================================
@app.route("/api/latest", methods=["GET"])
def get_latest():
    try:
        db = get_db()
        try:
            with db.cursor(pymysql.cursors.DictCursor) as cur:
                cur.execute("SELECT * FROM sensor_logs ORDER BY id DESC LIMIT 1")
                row = cur.fetchone()

            if not row:
                return jsonify({"message": "Belum ada data"}), 404

            if "waktu" in row and row["waktu"]:
                row["waktu"] = row["waktu"].strftime("%Y-%m-%d %H:%M:%S")
            return jsonify(row)
        finally:
            db.close()
    except Exception as e:
        return jsonify({"error": "Gagal koneksi MySQL", "details": str(e)}), 500

# =============================================================
# 3. READ -> history buat grafik Chart.js
# =============================================================
@app.route("/api/history", methods=["GET"])
def get_history():
    try:
        limit = request.args.get("limit", default=50, type=int)
        db = get_db()
        try:
            with db.cursor(pymysql.cursors.DictCursor) as cur:
                cur.execute("SELECT * FROM sensor_logs ORDER BY id DESC LIMIT %s", (limit,))
                rows = cur.fetchall()

            rows.reverse()
            for r in rows:
                if "waktu" in r and r["waktu"]:
                    r["waktu"] = r["waktu"].strftime("%Y-%m-%d %H:%M:%S")

            return jsonify(rows)
        finally:
            db.close()
    except Exception as e:
        return jsonify({"error": "Gagal koneksi MySQL", "details": str(e)}), 500

# =============================================================
# READ -> Data Log Harian Berdasarkan Tab Day 1-5
# =============================================================
@app.route("/api/logs_harian", methods=["GET"])
def get_logs_harian():
    try:
        day_id = request.args.get("day", default="day1")
        hari_ini = datetime.now().date()
        
        days_map = {"day2": 1, "day3": 2, "day4": 3, "day5": 4}
        sub_days = days_map.get(day_id, 0)
        target_date = hari_ini - timedelta(days=sub_days)

        db = get_db()
        try:
            with db.cursor(pymysql.cursors.DictCursor) as cur:
                cur.execute(
                    "SELECT * FROM sensor_logs WHERE DATE(waktu) = %s ORDER BY waktu DESC",
                    (target_date,)
                )
                rows = cur.fetchall()

            for r in rows:
                if "waktu" in r and r["waktu"]:
                    r["waktu"] = r["waktu"].strftime("%Y-%m-%d %H:%M:%S")

            return jsonify(rows)
        finally:
            db.close()
    except Exception as e:
        return jsonify({"error": "Gagal koneksi MySQL", "details": str(e)}), 500

# =============================================================
# 4 & 5. READ/UPDATE -> settings
# =============================================================
@app.route("/api/settings", methods=["GET"])
def get_settings():
    try:
        db = get_db()
        try:
            with db.cursor(pymysql.cursors.DictCursor) as cur:
                cur.execute("SELECT * FROM settings WHERE id = 1")
                row = cur.fetchone()
            return jsonify(row if row else {})
        finally:
            db.close()
    except Exception as e:
        return jsonify({"error": "Gagal koneksi MySQL", "details": str(e)}), 500

@app.route("/api/settings", methods=["POST"])
def update_settings():
    try:
        data = request.get_json(force=True, silent=True)
        if not data:
            return jsonify({"error": "Data JSON tidak valid"}), 400

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
    except Exception as e:
        return jsonify({"error": "Gagal update settings", "details": str(e)}), 500

# =============================================================
# 6. EXPORT -> Unduh CSV
# =============================================================
@app.route("/api/export", methods=["GET"])
def export_csv():
    try:
        db = get_db()
        try:
            df = pd.read_sql("SELECT * FROM sensor_logs ORDER BY id ASC", db)
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
    except Exception as e:
        return jsonify({"error": "Gagal export CSV", "details": str(e)}), 500

# =============================================================
# 7. DELETE -> Hapus Data Log
# =============================================================
@app.route("/api/logs", methods=["DELETE"])
def delete_old_logs():
    try:
        before = request.args.get("before")
        delete_all = request.args.get("all")

        db = get_db()
        try:
            with db.cursor() as cur:
                if delete_all == "true":
                    cur.execute("DELETE FROM sensor_logs")
                elif before:
                    cur.execute("DELETE FROM sensor_logs WHERE waktu < %s", (before,))
                else:
                    return jsonify({"error": "Parameter 'before' atau 'all' wajib diisi"}), 400

                deleted_count = cur.rowcount
            return jsonify({"status": "ok", "deleted_rows": deleted_count})
        finally:
            db.close()
    except Exception as e:
        return jsonify({"error": "Gagal hapus log", "details": str(e)}), 500

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
