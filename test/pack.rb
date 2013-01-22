assert('Array#pack exists') do
  a = []
  a.respond_to? :pack
end

assert('String#unpack exists') do
  s = ""
  s.respond_to? :unpack
end

assert("Directive 'C' test") do
  a = [97, 0, 98, 99, 100]
  s = a.pack('CCCCC')
  s.unpack('CCCCC') == a
end

assert("Directive 'S' test") do
  a = [1234, 42, 0, 65535]
  a.pack('SSSS').unpack('SSSS') == a
end

assert("Directive 'L' test") do
  a = [0, 65536, 100000, 2147483647]
  a.pack('LLLL').unpack('LLLL') == a
end

assert("Directive 'Q' test") do
  if 2147483648.class == Fixnum
    # Only test this when 64-bit fixnum is available
    a = [0, 2147483648, 3007, 100000000000]
    a.pack('QQQQ').unpack('QQQQ') == a
  else
    true
  end
end

assert("'c' test") do
  a = [100, 0, -1, -3]
  a.pack('cccc').unpack('cccc') == a
end

assert("'s' test") do
  a = [0, 32767, -10]
  a.pack('sss').unpack('sss') == a
end

assert("'l' test") do
  # For mruby, -2147483648 is actually float, maybe this has to
  # do with the parser?
  a = [0, 65536, -2147483647, 2147483647]
  a.pack('llll').unpack('llll') == a
end

assert("Directive 'q' test") do
  if 2147483648.class == Fixnum
    # Only test this when 64-bit fixnum is available
    a = [0, 2147483648, -3007, 100000000000]
    a.pack('qqqq').unpack('qqqq') == a
  else
    true
  end
end
