class CreateReadings < ActiveRecord::Migration
  def self.up
    create_table(:readings) do |t|
      t.column :code_point, :integer
      t.column :reading_type, :integer
      t.column :reading, :string
    end
    add_index :readings, :code_point
  end

  def self.down
    drop_table :readings
  end
end
