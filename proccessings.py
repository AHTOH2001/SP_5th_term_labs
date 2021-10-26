from config import data_file_name, portion_size, max_number
from time import sleep, time
from multiprocessing import Pipe, Pool, Process, SimpleQueue, Lock
import string
from multiprocessing.connection import wait
from collections import deque


def puzir_sort(data):
    # return sorted(data)
    cdata = data.copy()
    for i in range(len(cdata)):
        for j in range(i + 1, len(cdata)):
            if cdata[i] > cdata[j]:
                cdata[i], cdata[j] = cdata[j], cdata[i]
    return cdata


def naive():
    start_time_read = time()
    print('Naive is reading...')
    with open(data_file_name, 'r', buffering=1) as fp:
        data = [int(e) for e in fp.read().split()]

    print(
        f'Naive have read and parse for {round(time() - start_time_read, 2)}s')

    start_time_sort = time()
    print('Naive is sorting...')
    data = puzir_sort(data)
    print(f'Naive have sorted for {round(time() - start_time_sort, 2)}s')

    print(f'In total naive took {round(time() - start_time_read, 2)}s')

    return data


def task_sort(data, id, w):
    start_time = time()
    print(f'Smart started sorting part {id}...\n', end='')
    data = [int(e) for e in data.strip().split()]
    res = puzir_sort(data)
    w.send(res)
    w.close()

    print(
        f'Smart have sorted part {id} for {round(time() - start_time, 2)}s\n', end='')
    return res


def smart():
    start_time_read = time()
    print('Smart started reading...')
    readers = []
    k = 0
    with open(data_file_name, 'r', buffering=1) as fp:
        while True:
            raw_portion = fp.read(portion_size)
            if raw_portion == '':
                break
            while (True):
                ch = fp.read(1)
                if ch in string.whitespace:
                    break
                else:
                    raw_portion += ch
            k += 1
            r, w = Pipe(duplex=False)
            readers.append(r)
            process = Process(target=task_sort, args=(raw_portion, k, w))
            process.start()
            # We close the writable end of the pipe now to be sure that
            # p is the only process which owns a handle for it.  This
            # ensures that when p closes its handle for the writable end,
            # wait() will promptly report the readable end as being ready.
            w.close()

    print(f'Smart have read for {round(time() - start_time_read, 2)}s')

    sorted_portions = []
    while readers:
        for r in wait(readers):
            try:
                msg = r.recv()
            except EOFError:
                readers.remove(r)
            else:
                sorted_portions.append(deque(msg))

    print(f'All processes ended for {round(time() - start_time_read, 2)}s')

    start_time_merge = time()
    res = []
    while True:
        min_i = -1
        for i in range(len(sorted_portions)):
            if min_i == -1 or sorted_portions[min_i][0] > sorted_portions[i][0]:
                min_i = i

        if (min_i == -1):
            break
        res.append(sorted_portions[min_i].popleft())
        if (len(sorted_portions[min_i]) == 0):
            sorted_portions.pop(min_i)
    print(
        f'All portions are merged for {round(time() - start_time_merge, 2)}s')

    print(f'In total smart took {round(time() - start_time_read, 2)}s')

    return res
