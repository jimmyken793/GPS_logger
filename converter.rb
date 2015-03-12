#!/usr/bin/env ruby

require 'rubygems'
require 'optparse'
require 'gpx'

options = {
	:verbose => false
}
op = OptionParser.new do |opts|
	opts.banner = "Usage: #{$0} input_path [options]"
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

if ARGV.size<1
	puts op
	exit
end

include GPX

def distance loc1, loc2
	rad_per_deg = Math::PI/180  # PI / 180
	rkm = 6371                  # Earth radius in kilometers
	rm = rkm * 1000             # Radius in meters

	dlat_rad = (loc2.lat-loc1.lat) * rad_per_deg  # Delta, converted to rad
	dlon_rad = (loc2.lon-loc1.lon) * rad_per_deg

	lat1_rad, lon1_rad = [loc1.lat, loc1.lon].map {|i| i * rad_per_deg }
	lat2_rad, lon2_rad = [loc2.lat, loc2.lon].map {|i| i * rad_per_deg }

	a = Math.sin(dlat_rad/2)**2 + Math.cos(lat1_rad) * Math.cos(lat2_rad) * Math.sin(dlon_rad/2)**2
	c = 2 * Math::atan2(Math::sqrt(a), Math::sqrt(1-a))

	rm * c # Delta in meters
end

def new_track
	$track = Track.new
	$segment = Segment.new
	$track.append_segment($segment)
	$tracks << $track
end

def convert(input_file, output_file, options)
	raw_data = IO.readlines(input_file)
	$tracks = []
	$track = nil
	$segment = nil




	new_track
	raw_data.each{|line|
		time, _, _, latitude, lat_dir, longitude, lon_dir, quality, satellites_num, horizontal_dilution, altitude, heicht_of_geoid, _, _, checksum = line.split(',')
		satellites_num = satellites_num.to_i
		if satellites_num.to_i > 0
			time = Time.at(time.to_i)
			altitude = altitude.to_f
			latitude = latitude[0,2].to_f + (latitude[2,latitude.length-2].to_f / 60.0)
			lat_dir == 'N' ? latitude : -latitude
			longitude = longitude[0,3].to_f + (longitude[3,longitude.length-2].to_f / 60.0)
			lon_dir == 'E' ? longitude : -longitude
			p = TrackPoint.new(:lat => latitude, :lon => longitude, :elevation => altitude, :time => time)
			if $segment.latest_point
				d = distance($segment.latest_point, p)
				if d > 3000
					# puts "distance: #{d} meters"
					new_track
				end
			end
			$segment.append_point(p)
		else
			# puts "End of segment: #{segment.points.size} points" if !segment.nil?
			# track = nil
			# segment = nil
		end
	}
	i=0
	if $tracks.size > 1
		$tracks.each{|track|
			gpx_file = GPXFile.new(:tracks => [track])
			gpx_file.write("#{i}_#{output_file}")
			i = i + 1
		}
	else
		gpx_file = GPXFile.new(:tracks => $tracks)
		gpx_file.write(output_file)
	end
end

Dir.glob("#{ARGV[0]}/GPS*.txt").each{|filename|
	converted_filename = "#{File.dirname(filename)}/#{File.basename(filename,File.extname(filename))}.gpx"
	puts "converting #{filename} => #{converted_filename}"
	convert(filename, converted_filename, options)
}

