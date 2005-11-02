#!/usr/bin/ruby

require 'gtk2'
require 'dictionary'

class Gtk2DictView
  def initialize
    @dict = Dictionary.new
    create_gui
  end

  def create_gui
    Gtk.init

    window = Gtk::Window.new
    window.signal_connect("delete_event") {
      #true
      false
    }

    window.signal_connect("destroy") {
      Gtk.main_quit
    }

    box1 = Gtk::VBox.new(false, 15)

    @area = Gtk::DrawingArea.new
    @area.set_size_request(300, 300)
    @area.signal_connect("expose_event") {
      draw_letter
    }

    box2 = Gtk::HBox.new(false, 5)

    box1.add(@area)
    box1.add(box2)

    @stroke_count_combobox = Gtk::ComboBox.new
    for i in 1 .. @dict.letters_by_stroke_count.size
      @stroke_count_combobox.append_text(i.to_s)
    end

    @stroke_count_combobox.signal_connect("changed") {
      update_letter_liststore
    }

    stroke_count_label = Gtk::Label.new(GLib.convert("²è", "utf-8", "euc-jp"))

    @letter_liststore = Gtk::ListStore.new(String)
    update_letter_liststore
    @letter_comboboxentry = Gtk::ComboBoxEntry.new(@letter_liststore, 0)
    @letter_comboboxentry.signal_connect("changed") {
      @letter = @letters[@letter_comboboxentry.active]
      draw_letter
    }
    @letter_comboboxentry.child.signal_connect("activate") {
      letter = @dict.get_letter(@letter_comboboxentry.child.text)
      if letter == nil
	dialog = Gtk::MessageDialog.new(window, Gtk::Dialog::MODAL,
					Gtk::MessageDialog::ERROR,
					Gtk::MessageDialog::BUTTONS_OK,
					"character not found in dictionary")
	dialog.run { |response| dialog.destroy }
      else
	@letter = letter
      end
      draw_letter
    }

    button = Gtk::Button.new(GLib.convert("½ªÎ»", "utf-8", "euc-jp"))
    button.signal_connect("clicked") {
      Gtk.main_quit
    }

    box2.pack_start(@stroke_count_combobox, false, false, 0)
    box2.pack_start(stroke_count_label, false, false, 0)
    box2.pack_start(@letter_comboboxentry, true, true, 0)
    box2.pack_start(button, false, false, 0)

    window.border_width = 10
    window.add(box1)
    window.show_all
  end

  def update_letter_liststore
    stroke_count = @stroke_count_combobox.active
    @letters = @dict.letters_by_stroke_count[stroke_count]

    @letter_liststore.clear
    for letter in @letters
      @letter_liststore.append[0] = letter.character
    end
  end

  def stroke_to_points(stroke)
    points = []
    for point in stroke.points
      points.push([point.x, point.y])
    end
    return points
  end

  def draw_letter
    if @letter == nil
      return
    end

    @area.window.clear
    for stroke in @letter.strokes
      points = stroke_to_points(stroke)
      @area.window.draw_lines(@area.style.fg_gc(@area.state), points)
    end
  end
end

dv = Gtk2DictView.new
Gtk.main
