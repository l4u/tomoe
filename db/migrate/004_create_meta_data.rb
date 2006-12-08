class CreateMetaData < ActiveRecord::Migration
  def self.up
    create_table(:meta_data) do |t|
      t.column :code_point, :integer
      t.column :key, :string
      t.column :value, :string
    end
    add_index :meta_data, :code_point
  end

  def self.down
    drop_table :meta_data
  end
end
