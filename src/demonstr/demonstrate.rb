class Demonstrate
  def loop
    p Demonstrate.logic_data.map { |i| i.to_s 16 }
  end
end

def main
  Demonstrate.new.loop
rescue Exception => e
  s = ''
  s << "Exception caught: #{ e }"
  s << e.backtrace.join("\n")

  puts "\n#{ s.gsub(/^/, "\t\t") }\n\n"
end
