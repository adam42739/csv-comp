import cboescraper
import datetime
import subprocess


BASE = "files/"


def download_tickers(tickers: list[str], downloads: str):
    cboescraper.save_tickers(tickers, downloads, BASE, datetime.datetime.today())


EXE_PATH = "release/out/build/opt_file_comp"
FULL_BASE_PATH = "/Users/adamlynch/Documents/HobbyWork/csv-comp/files/"


def compress_ticker(ticker: str, date: datetime.datetime):
    csv_path = (
        FULL_BASE_PATH
        + ticker
        + "="
        + datetime.datetime.strftime(date, "%Y-%m-%d")
        + ".csv"
    )
    comp_path = (
        FULL_BASE_PATH
        + ticker
        + "="
        + datetime.datetime.strftime(date, "%Y-%m-%d")
        + ".csvc"
    )
    subprocess.run([EXE_PATH, "-c", comp_path, csv_path])


def decompress_ticker(ticker: str, date: datetime.datetime):
    csv_path = (
        FULL_BASE_PATH
        + ticker
        + "="
        + datetime.datetime.strftime(date, "%Y-%m-%d")
        + "-dc"
        + ".csv"
    )
    comp_path = (
        FULL_BASE_PATH
        + ticker
        + "="
        + datetime.datetime.strftime(date, "%Y-%m-%d")
        + ".csvc"
    )
    subprocess.run([EXE_PATH, "-dc", comp_path, csv_path])


DOWNLOADS_PATH = "/Users/adamlynch/Downloads/"

# download_tickers(["aapl"], DOWNLOADS_PATH)
# compress_ticker("aapl", datetime.datetime.today())
decompress_ticker("aapl", datetime.datetime.today())
