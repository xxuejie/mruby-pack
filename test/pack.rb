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
  a.pack('CCCCC').unpack('CCCCC') == a
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
  a = [0, 2147483648, 3007, 100000000000]
  a.pack('QQQQ').unpack('QQQQ') == a
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
  a = [0, 65536, -2147483648, 2147483647]
  a.pack('llll').unpack('llll') == a
end

assert("Directive 'q' test") do
  a = [0, 2147483648, -3007, 100000000000]
  a.pack('qqqq').unpack('qqqq') == a
end

assert("'*' test") do
  a = [1, 2, 3, 4, 5, 0, -1, -2]
  a.pack('s*').unpack('s*') == a
end

assert("'*' must follow some directives in pack") do
  ok = false

  begin
    [1, 2].pack('*')
  rescue ArgumentError => e
    ok = true if e.message == "'*' must follow existing directives!"
  end

  ok
end

assert("'*' must follow some directives in unpack") do
  ok = false

  begin
    "abc".unpack('*')
  rescue ArgumentError => e
    ok = true if e.message == "'*' must follow existing directives!"
  end

  ok
end
