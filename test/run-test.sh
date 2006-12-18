#!/bin/sh

base=`dirname $0`
make -C $base/.. > /dev/null || exit 1

RUBY=`(cd $base && make output_RUBY)`
for dict_module in `(cd $base && make output_dict_modules)`; do
    echo "tests for $dict_module"
    TOMOE_DICT_MODULE="$dict_module" $RUBY $base/run-spec.rb
done
