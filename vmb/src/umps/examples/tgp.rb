#!/usr/bin/ruby

n=true;
b=Array.new
c=Array.new
foo=ARGV.shift
while l=gets
   if n
   	n = false
   else
   	n = true
   end
   a = l.split(" = ");
   b.push(a) if(n)
   c.push(a) if(!n)
end

if(foo != nil)
b.each do |x|
   puts x[0].chomp + " " + x[1].chomp
end
else
c.each do |x|
   puts x[0].chomp + " " + x[1].chomp
end
end
