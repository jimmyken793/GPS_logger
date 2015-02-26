#!/usr/bin/env ruby

require 'rubygems'
require 'optparse'
require 'gpx'

options = {
	:verbose => false
}
op = OptionParser.new do |opts|
	opts.banner = "Usage: #{$0} input_file output_file [options]"
	opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
		options[:verbose] = true
	end
end

begin
	op.parse!
rescue => e
	puts e
	puts op
	exit
end

if ARGV.size<2
	puts op
	exit
end

include GPX

raw_data = IO.readlines(ARGV[0])
track = Track.new
segment = Segment.new
raw_data.each{|line|
	satellites_num, latitude, longitude, hour, minute, second = line.split(/[,: ]+/)
	if satellites_num.to_i > 0
		latitude = latitude[0..latitude.length-8].to_i + (latitude[latitude.length-7..-1].to_f / 0.6 / 100)
		longitude = longitude[0..longitude.length-8].to_i + (longitude[longitude.length-7..-1].to_f / 0.6 / 100)
		p = TrackPoint.new(:lat => latitude, :lon => longitude, :elevation => 0.0, :time => Time.now)
		segment.append_point(p)
	end
}
track.append_segment(segment)
gpx_file = GPXFile.new(:tracks => [track])
gpx_file.write(ARGV[1])