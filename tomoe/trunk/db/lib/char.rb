class Char < ActiveRecord::Base
  set_primary_key :code_point
  has_many :readings, :foreign_key => "code_point"
  has_many :radicals, :foreign_key => "code_point"
  has_many :meta_data, :foreign_key => "code_point"
end
