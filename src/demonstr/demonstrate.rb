class Demonstrate
  def initialize
    @sounds = {
      1 => "1C.ogg",
      5 => "2D.ogg",
      9 => "3E.ogg",
      13 => "4F.ogg",
      17 => "5G.ogg",
      21 => "6A.ogg",
      25 => "7B.ogg",
      29 => "8C.ogg",
      32 => "9D.ogg",
    }
  end

  def run
    old = {}
    keep_frames = 4

    loop do
      data = Demonstrate.logic_data
      new  = []

      data.each do |sample|
        unless old[sample]
          old[sample] = keep_frames
        else
          new << sample
          if (sound = @sounds[sample])
            %x{mplayer data/#{ sound } &> /dev/null < /dev/null &}
          end
        end
      end

      old.each do |sample, count|
        old[sample] -= 1
        old.delete(sample) if count == 1
      end

      p new
    end
  rescue Interrupt
    Demonstrate.fin
  end
end

def main
  Demonstrate.new.run
rescue Exception => e
  s = ''
  s << "#{ e.class }: #{ e }\n"
  s << e.backtrace.join("\n")

  puts "\n#{ s.gsub(/^/, "\t\t") }\n\n"

  Demonstrate.fin
end
