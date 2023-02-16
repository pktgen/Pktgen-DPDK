# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) <2019-2023> Intel Corporation

doc_targets = []
doc_target_names = []

# Take out API doxygen for now.
subdir('api')

subdir('source')

if doc_targets.length() == 0
    message = 'No docs targets found'
else
    message = 'Building docs:'
endif
run_target('doc', command: ['echo', message, doc_target_names],
    depends: doc_targets)

