#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Generate a large input file based on the answer
#

import sys
import string
import random

MAX_URL_LEN = 1024


def gen_random_word():
    size = random.randint(1, 20)
    return ''.join([random.choice(string.letters) for i in range(size)])


def gen_random_url():
    url = 'http://www.%s.com?' % gen_random_word()
    length = len(url)
    final_length = random.randint(length + 1, MAX_URL_LEN)
    while length < final_length:
        params = '%s=%s' % (gen_random_word(), gen_random_word())
        if len(params) + length > final_length:
            break
        url += params
        length += len(params)
    return url


def gen_random_urls_file(file_prefix, file_size, url_cnt):
    # Total length if we repeat all the given URLs separating by '\n'
    given_urls_remaining_len = sum(map(lambda (url, cnt): len(url) * cnt + 1, url_cnt.items()))
    # The ratio decides whether to use a given URL or generate a random one
    ans_ratio = float(given_urls_remaining_len) / file_size
    remaining_size = file_size
    with open(file_prefix + '_input.txt', 'w') as res_file:
        while remaining_size > 0:
            if given_urls_remaining_len >= remaining_size:
                # Don't need to generate random URLs now. Just fulfill our given URLs
                for url in url_cnt:
                    while url_cnt[url] > 0:
                        res_file.write(url + '\n')
                        url_cnt[url] -= 1
                break
            if random.random() < ans_ratio:
                url = random.choice(url_cnt.keys())
                given_urls_remaining_len -= len(url) + 1
                if url_cnt[url] == 1:   # last time
                    url_cnt.pop(url)
                else:
                    url_cnt[url] -= 1
            else:
                url = gen_random_url()
            res_file.write(url + '\n')
            remaining_size -= len(url) + 1

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print('Too few parameters!\nArgs: ans_file file_size_in_gb target_dir')
        exit(1)
    ans_file = sys.argv[1]
    file_size = int(float(sys.argv[2]) * 1024 * 1024 * 1024)
    target_dir = sys.argv[3]
    res = dict()
    with open(ans_file) as ans:
        for line in ans:
            url, cnt = line.strip().split()
            res[url] = int(cnt)
    print('Generating input file...')
    gen_random_urls_file(target_dir + '/' + ans_file, file_size, res)
    print('Done')
