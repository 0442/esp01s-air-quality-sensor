#!/bin/env python3

import socket
import sqlite3
from time import sleep
from sys import argv

ESP_IP = "192.168.0.0"
ESP_PORT = 5505
DATA_FETCH_INTERVAL = 8  # pm1006 response time is 8 seconds.


class PMLogger:
    def __init__(self, file_name: str, logging_period_days: float = "30") -> None:
        self._db_file_name = file_name
        self._logging_period = logging_period_days
        self._db = self._init_db()

    def _init_db(self) -> sqlite3.Connection:
        db = sqlite3.connect(self._db_file_name)
        db.execute("CREATE TABLE IF NOT EXISTS "
                   "logs (value INTEGER, timestamp TIME);")
        db.execute("CREATE INDEX IF NOT EXISTS "
                   "idx_timestamp ON logs (timestamp);")

        return db

    def log_reading(self, pm_value: int) -> None:
        sql_insert = """
            INSERT INTO logs (
                value,
                timestamp
            )
            VALUES (
                ?,
                strftime('%s')
            );
        """
        self._db.execute(sql_insert, [pm_value])
        self._remove_expired_readings()
        self._db.commit()

    def _remove_expired_readings(self):
        sql_del_old = """
            DELETE FROM logs
            WHERE julianday('now') - julianday(timestamp) > ?;
        """
        self._db.execute(sql_del_old, [self._logging_period])

    def close(self) -> None:
        self._db.commit()
        self._db.close()
        print("Database connection closed.")

    def clear_logs(self) -> None:
        self._db.execute("DELETE FROM logs;")
        print("All logs cleared.")

    @property
    def log_length(self):
        count = self._db.execute("SELECT count(*) FROM logs;").fetchone()
        return count[0]


class EspConnection:
    def __init__(self, ip, port) -> None:
        self._ip = ip
        self._port = port
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.settimeout(1)
        self._data_fetch_msg = b'foo'

    def get_reading(self) -> int:
        self._sock.sendto(self._data_fetch_msg, (self._ip, self._port))
        raw_data, _addr = self._sock.recvfrom(1024)
        pm_reading = int(raw_data.hex(), base=16)
        return pm_reading


def loop(pmlogger: PMLogger):
    vindriktning = EspConnection(ESP_IP, ESP_PORT)

    while True:
        pm_reading = None

        try:
            pm_reading = vindriktning.get_reading()
        except TimeoutError:
            print("Request to esp timed out.")
        except Exception as e:
            print("Error while fetchin pm reading:", e.with_traceback())

        if pm_reading is None:
            continue

        pmlogger.log_reading(pm_reading)

        print("PM2.5:", pm_reading, "data points:", pmlogger.log_length)
        sleep(DATA_FETCH_INTERVAL)


def main():
    pmlogger = PMLogger("logs.db")
    try:
        if len(argv) >= 2 and argv[1] == "clear-all":
            pmlogger.clear_logs()
            pmlogger.close()
            return

        print("To clear all logs, run with argument 'clear-all'.")
        loop(pmlogger)

    finally:
        pmlogger.close()


if __name__ == "__main__":
    main()
