#!/usr/bin/env ruby

#
# Find blocks associated with a given file.
#
# example: path-to-blocks.sh /test-warehouse/tpcds1000gb.store_sales/ss_date=2003-01-02 
#                            /test-warehouse/tpcds1000gb.store_sales/ss_date=2003-01-03 > /tmp/blocks.txt
#

require 'open3'

ARGV.each do |file|
  cmd = "/home/cmccabe/h/bin/hadoop fsck '#{file}' -files -blocks -locations 2>&1"
  IO.popen(cmd, "r") do |io|
    io.readlines.each do |line| 
      # example line:
      # 0. BP-897330414-10.20.212.20-1378404695701:blk_1073742166_1342 len=134217728 repl=3 [10.20.212.12:6100, 10.20.212.16:6100, 10.20.212.14:6100]
      if (line =~ /BP-897330414-10.20.212.20-1378404695701:blk_([0-9]*)_/) then
        print "blk_#{$1}\n"
      end
    end
    io.close
  end
end
exit 0
