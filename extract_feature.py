#!/usr/bin/env python
# coding=utf-8
import sys
import datetime
import json
import time
import random
from pyspark.sql import SparkSession
from pyspark import SparkContext
import libpyfeature_extract
fe = libpyfeature_extract.PyFeatureExtract('')

def run_date(date, dates):
    ss = SparkSession.builder.appName("ads_feature-extract").enableHiveSupport().getOrCreate()
    input_path = ''
    output_path = ''
    ss.read(input_path).map(fe.extract).filter(lambda x: x and x != '{}')..saveAsTextFile(output_path)


if __name__ == '__main__':
    run_date(sys.argv[1])
