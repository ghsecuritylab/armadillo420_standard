# encoding: UTF-8

# This file contains data derived from the IANA Time Zone Database
# (http://www.iana.org/time-zones).

module TZInfo
  module Data
    module Definitions
      module Etc
        module GMT__m__11
          include TimezoneDefinition
          
          timezone 'Etc/GMT-11' do |tz|
            tz.offset :o0, 39600, 0, :'GMT-11'
            
          end
        end
      end
    end
  end
end
