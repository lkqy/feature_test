#!/usr/bin/env python
# coding=utf-8
import sys
import datetime
import json
import time
import random
import libpyfeature_extract
from pyspark.sql import SparkSession
from pyspark import SparkContext
fe = libpyfeature_extract.PyFeatureExtract('')
#print fe.extract('{}')

def run_date(date, dates):
    ss = SparkSession.builder.appName("ads_feature-extract").enableHiveSupport().getOrCreate()
    input_path = ''
    output_path = ''
    ss.read(input_path).map(fe.extract).filter(lambda x: x and x != '{}').saveAsTextFile(output_path)


if __name__ == '__main__':
    run_date(sys.argv[1])
