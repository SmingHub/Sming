require "tempfile"

passing = true

Dir["#{__dir__}/cases/*"].each do |test_case|
  test_name = test_case.split("/").last

  input, expected = File.read(test_case).split("\n\n")

  begin
    output_file = "/tmp/ws_parser-#{$$}-output"
    expected_file = "/tmp/ws_parser-#{$$}-expected"

    io = IO.popen(["#{__dir__}/parse"], "wb", :out => output_file)

    input.lines.each_with_index do |chunk, index|
      io.syswrite(eval(chunk, nil, test_case, index + 1).b)
    end

    io.close

    if File.read(output_file) == expected
      print "."
      $stdout.flush
    else
      File.write(expected_file, expected)
      puts
      puts "Failed: #{test_name}"
      puts IO.popen(["diff", "-u", expected_file, output_file]).readlines.drop(2).join
      puts
      passing = false
    end
  ensure
    File.delete(output_file) rescue nil
    File.delete(expected_file) rescue nil
  end
end

puts
exit passing
