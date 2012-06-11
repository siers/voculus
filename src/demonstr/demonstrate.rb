class Demonstrate
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
end
