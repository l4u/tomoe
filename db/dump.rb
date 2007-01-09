#!/usr/bin/env ruby

base = File.expand_path(File.dirname(__FILE__))
require File.join(base, "init")

module ActiveRecord
  module ConnectionAdapters #:nodoc:
    class SQLiteAdapter < AbstractAdapter
      def structure_dump #:nodoc:
        sql = <<-SQL
          SELECT sql
          FROM (SELECT * FROM sqlite_master UNION ALL
                SELECT * FROM sqlite_temp_master)
          WHERE type!='meta' AND
                sql NOTNULL AND
                name NOT LIKE 'sqlite_%'
          ORDER BY substr(type,2,1), name
        SQL

        execute(sql).inject("") do |structure, row|
          structure + "#{row[0]};\n\n"
        end
      end
    end
  end
end

file = File.join(base, "dump.sql")
connection = ActiveRecord::Base.connection
adapter = connection.adapter_name.downcase
case adapter
when "mysql", "oci", "sqlite", "sqlite3"
  File.open(file, "w+") do |f|
    f << connection.structure_dump
  end
else
  raise "dump is not supported by #{adapter}"
end
