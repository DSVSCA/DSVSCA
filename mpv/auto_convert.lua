mputils = require 'mp.utils'

function start_conversion_and_load_3d_audio()
   print("Switching over to 3d audio and restarting video!!");
   local file_path = mp.get_property("path", "")
   local curr_time_into = mp.get_property("time-pos","")
   local sofa_file = "~/Videos/SurroundVideos/subject_119.sofa"
   local command = "~/coding/DSVSCA/example/DSVSCA -v " .. file_path .. " -s " .. sofa_file  
   -- local command = "~/coding/dsvsca/DSVSCA/example/DSVSCA -v " .. path .. " -s " .. sofa_file .. " &"
   -- Asynch version above, add a call to sleep(3 or so) to give it time to make header.
   os.execute(command)
   local filename_to_open = file_path:gsub("%.([%w%-]+)$", "") .. "-virtualized".. ".mp4"

   local curr_time_into = mp.get_property("time-pos","")
   mp.commandv("loadfile", filename_to_open, "replace", "start=+"..curr_time_into) -- Remove curr_time_into if doing asynch
   mp.set_property_native("pause", false) 
end

function sleep(n)
  os.execute("sleep " .. tonumber(n))
end

mp.add_key_binding("alt+w", "3D_AUDIOIFY", start_conversion_and_load_3d_audio)

