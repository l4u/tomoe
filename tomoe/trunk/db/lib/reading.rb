class Reading < ActiveRecord::Base
  belongs_to :char, :foreign_key => "code_point"
end
