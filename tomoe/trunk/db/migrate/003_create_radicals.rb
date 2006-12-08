class CreateRadicals < ActiveRecord::Migration
  def self.up
    create_table(:radicals) do |t|
      t.column :code_point, :integer
      t.column :radical_code_point, :integer
    end
    add_index :radicals, :code_point
  end

  def self.down
    drop_table :radicals
  end
end
