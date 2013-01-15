assert('Array#pack exists') do
  a = []
  a.respond_to? :pack
end

assert('String#unpack exists') do
  s = ""
  s.respond_to? :unpack
end

assert("Directive 'C' test") do
  a = [1, 2, 3, 4]
  s = a.pack('C*')
  s.unpack('C*') == a
end

