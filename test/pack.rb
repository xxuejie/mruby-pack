# Check if two arrays contains the same list of float values.
# Mruby may use either double or float to represent mrb_float,
# here we allow for customized tolerance for single-precision
# float test.
def check_floats(a, b, tolerance = Math::TOLERANCE)
  return false if a.length != b.length

  a.length.times do |i|
    f_a = a[i].to_f
    f_b = b[i].to_f

    equal = false
    if f_a.finite? and f_b.finite?
      equal = (f_a - f_b).abs < tolerance
    else
      equal = true
    end

    return false unless equal
  end

  true
end

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

assert("pack float") do
  a = [1.1, 2.34, 5.601, 100.001, 0.0, -7.123]
  # 'F' and 'f' should be the same
  check_floats(a.pack('FfFfFf').unpack('fFfFfF'), a, 1e-5)
end

assert("pack double") do
  a = [2.345678, 3.1415926535, -1.23456789, 0.00000001]
  check_floats(a.pack('DDdd').unpack('ddDD'), a)
end

assert("float with '*' test") do
  a = [1.1, 2.34, 5.601, 100.001, 0.0, -7.123]
  check_floats(a.pack('f*').unpack('f*'), a, 1e-5)
end

assert("test 'I' and 'i'") do
  a = [1, 2, -3, -4]
  a.pack('IIii').unpack("IIii") == a
end

assert("test !, _") do
  a = [1, 2, 3, 4, 5, 6, 7, -8, 9, -10, 11, -12]
  a.pack('S_S!I_I!L_L!s_s!i_i!l_l!').unpack('S_S!I_I!L_L!s_s!i_i!l_l!') == a
end

assert("test fixnum big endian") do
  [45, 1234, -90000].pack("S>L>q>") ==
    "\x00-\x00\x00\x04\xD2\xFF\xFF\xFF\xFF\xFF\xFE\xA0p"
end

assert("test fixnum little endian") do
  [-10, 1234, 65535].pack("s<l<Q<") ==
    "\xF6\xFF\xD2\x04\x00\x00\xFF\xFF\x00\x00\x00\x00\x00\x00"
end

assert("test float endian") do
  [1.23, -4.56, 9.87654321, -7.7890123456].pack("egGE") ==
    "\xA4p\x9D?\xC0\x91\xEB\x85@#\xC0\xCAE\x88\xF63\x10\xF81\xDA\xF2'\x1F\xC0"
end
