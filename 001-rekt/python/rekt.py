#!/usr/bin/env python3
"""
    rekt.py - rekt tester for python
    author: oxalorg
"""
import sys

from timeit import default_timer as timer
from importlib import import_module


# Set these constants
N = 100 # rect size, needed to find avg time per rect
GUID = '12345678-12345678-12345678-12345678-12345678'
POINT_FILE_PATH = GUID + '-points.txt'
REKT_FILE_PATH = GUID + '-rects.txt'
RES_FILE_PATH = GUID + '-results.txt'


def get_rekt():
    """
    Get rekt son

    :return: a list of rekts
    """
    with open(REKT_FILE_PATH) as fp:
        rekts = [tuple(map(float, rekt.strip().split())) for rekt in fp]
    return rekts


def init_user(mod):
    """
    Calls 'init' function from user module.
    We pass the path to file containing points
    """
    init = getattr(mod, 'init')
    init(POINT_FILE_PATH)


def run_user(mod, rekts):
    """
    Calls 'user' function from user module.
    We pass a list of rectangles, where each rectangle
    is represented by a tuple: (x1, x2, y1, y2)
    """
    run = getattr(mod, 'run')
    t_start = timer()
    run(rekts)
    t_end = timer()
    return t_end - t_start


def results_user(mod):
    """
    Calls 'results' function from user module.
    Your function must return a list of list of ranks
    eg: [[], [1.0, 5.0] ... ]
    """
    results = getattr(mod, 'results')
    return results()


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: ./rekt.py ox_submission.py')
        sys.exit()

    mod = import_module(sys.argv[1].rsplit('.')[0])
    rekts = get_rekt()

    init_user(mod)
    time_elapsed = run_user(mod, rekts)
    user_results = results_user(mod)

    # Validate if their ans is correct or not
    actual_results = []
    with open(RES_FILE_PATH) as fp:
        actual_results = [list(map(float, res.strip().split())) for res in fp]

    if user_results == actual_results:
        print('Avg time: ', time_elapsed / float(N))
        print('Ttl time: ', time_elapsed)
    else:
        print('Incorrect')
