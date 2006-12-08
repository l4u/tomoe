class CreateChars < ActiveRecord::Migration
  def self.up
    create_table(:chars, :primary_key => :code_point) do |t|
      t.column :code_point, :integer
      t.column :n_strokes, :integer
      t.column :variant, :integer
    end
    add_index :chars, :code_point
  end

  def self.down
    drop_table :chars
  end
end
