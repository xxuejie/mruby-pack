assert('Array#pack exists') do
  a = []
  a.respond_to? :pack
end

assert('String#unpack exists') do
  s = ""
  s.respond_to? :unpack
end

assert("Directive 'C' test") do
  a = [97, 98, 99, 100]
  s = a.pack('CCCC')
  s.unpack('CCCC') == a
end

assert("Directive 'S' test") do
  a = [1234, 42, 65535]
  a.pack('SSS').unpack('SSS') == a
end
