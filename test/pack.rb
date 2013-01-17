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
  # NOTE: in webruby, mruby is configured to only use 32-bit int.
  # This due to limitations of emscripten(which then goes to JS).
  # So anything bigger than 2147483647 or smaller than -2147483648
  # are not supported yet.
  a = [0, 65536, 100000, 2147483647]
  a.pack('LLLL').unpack('LLLL') == a
end
