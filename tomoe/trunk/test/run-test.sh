#!/bin/sh

make -C `dirname $0`/.. > /dev/null && \
  `dirname $0`/run-spec.rb
