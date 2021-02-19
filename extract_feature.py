#!/usr/bin/env python
# coding=utf-8
from multiprocessing import Process
import sys
import datetime
import ujson as json
import time
import random
import libpyfeature_extract
import tensorflow as tf


fe = libpyfeature_extract.PyFeatureExtract('')

def generator(data):
    x = fe.extract(data)
    obj = json.loads(x)
    ikv = {}
    for k, v in obj.get('int_features', {}).items():
        ikv[k] = tf.train.Feature(int64_list=tf.train.Int64List(value=[v]))
    for k in ['imp', 'click', 'install', 'attr_install']:
        v = obj.get('label', {}).get(k, 0)
        ikv['is_' + k] = tf.train.Feature(int64_list=tf.train.Int64List(value=[v]))

    for k, v in obj.get('float_features', {}).items():
        ikv[k] = tf.train.Feature(float_list=tf.train.FloatList(value=[v]))

    for k, v in obj.get('sequence_features', {}).items():
        ikv[k] = tf.train.Feature(int64_list=tf.train.Int64List(value=v))

    example_proto = tf.train.Example(features=tf.train.Features(feature=ikv))
    return example_proto.SerializeToString()

def process(in_path):
    out_path = in_path.replace('train_data', 'ads_train_data')
    print(in_path, out_path)
    filename = out_path
    options_zlib = tf.python_io.TFRecordOptions(tf.python_io.TFRecordCompressionType.GZIP)
    writer = tf.python_io.TFRecordWriter(filename, options=options_zlib)
    cnt = 0
    with open(in_path) as f:
        for line in f:
            cnt += 1
            s = generator(line)
            writer.write(s)
            if cnt % 1000 == 0:
                print('process %s' % cnt)
    writer.close()


def batch(paths):
    print(paths)
    x = []
    for in_path in paths:
        p = Process(target=process, args=(in_path,))
        p.start()
        x.append(p)

    for p in x:
        p.join()


if __name__ == '__main__':
    size = 200
    for x in range(1, len(sys.argv[1:])+size-1, size):
        batch(sys.argv[x:x+size])
