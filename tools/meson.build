# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) <2019-2024> Intel Corporation

# set up map-to-def script using python, either built-in or external
python3 = import('python').find_installation(required: false)
if python3.found()
    py3 = [python3]
else
    py3 = ['meson', 'runpython']
endif
sphinx_wrapper = py3 + files('call-sphinx-build.py')

