import cboescraper
import datetime
import subprocess
import pandas
import os


TICKERS = ["aapl", "amzn", "nxpi", "nvda", "msft", "goog", "meta", "jpm", "bac"]
DATE = datetime.datetime(2024, 9, 24)
BASE = "files/"
DOWNLOADS_PATH = "/Users/adamlynch/Downloads/"


def download_tickers():
    cboescraper.save_tickers(TICKERS, DOWNLOADS_PATH, BASE, datetime.datetime.today())


EXE_PATH = "release/out/build/csv_comp"
FULL_BASE_PATH = "/Users/adamlynch/Documents/HobbyWork/csv-comp/files/"


def compress_csvc(ticker: str):
    csv_path = (
        FULL_BASE_PATH
        + ticker
        + "="
        + datetime.datetime.strftime(DATE, "%Y-%m-%d")
        + ".csv"
    )
    comp_path = (
        FULL_BASE_PATH
        + ticker
        + "="
        + datetime.datetime.strftime(DATE, "%Y-%m-%d")
        + ".csvc"
    )
    subprocess.run([EXE_PATH, "-c", comp_path, csv_path])


def decompress_csvc(ticker: str):
    csv_path = (
        FULL_BASE_PATH
        + ticker
        + "="
        + datetime.datetime.strftime(DATE, "%Y-%m-%d")
        + "-dc"
        + ".csv"
    )
    comp_path = (
        FULL_BASE_PATH
        + ticker
        + "="
        + datetime.datetime.strftime(DATE, "%Y-%m-%d")
        + ".csvc"
    )
    subprocess.run([EXE_PATH, "-dc", comp_path, csv_path])


def compress():
    for ticker in TICKERS:
        compress_csvc(ticker)
        ticker_path = (
            FULL_BASE_PATH + ticker + "=" + datetime.datetime.strftime(DATE, "%Y-%m-%d")
        )
        df = pandas.read_csv(ticker_path + ".csv")
        df.to_hdf(ticker_path + ".h5", key="df")
        df.to_parquet(ticker_path + ".parquet")
        df.to_orc(ticker_path + ".orc")
        df.to_feather(ticker_path + ".feather")


def get_comp_ratio():
    bytes_csv = 0
    bytes_csvc = 0
    bytes_parquet = 0
    bytes_orc = 0
    bytes_feather = 0
    for ticker in TICKERS:
        ticker_path = (
            FULL_BASE_PATH + ticker + "=" + datetime.datetime.strftime(DATE, "%Y-%m-%d")
        )
        bytes_csv += os.path.getsize(ticker_path + ".csv")
        bytes_csvc += os.path.getsize(ticker_path + ".csvc")
        bytes_parquet += os.path.getsize(ticker_path + ".parquet")
        bytes_orc += os.path.getsize(ticker_path + ".orc")
        bytes_feather += os.path.getsize(ticker_path + ".feather")
    comp_csv = 100
    comp_csvc = 100 * bytes_csvc / bytes_csv
    comp_parquet = 100 * bytes_parquet / bytes_csv
    comp_orc = 100 * bytes_orc / bytes_csv
    comp_feather = 100 * bytes_feather / bytes_csv
    print("COMP CSV: ", round(comp_csv, 2))
    print("COMP CSVC: ", round(comp_csvc, 2))
    print("COMP PARQUET: ", round(comp_parquet, 2))
    print("COMP ORC: ", round(comp_orc, 2))
    print("COMP FEATHER: ", round(comp_feather, 2))
