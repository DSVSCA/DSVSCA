mputils = require 'mp.utils'

function start_conversion_and_load_3d_audio()
   print("Switching over to 3d audio and restarting video!!");
   local file_path = mp.get_property("path", "")
   local curr_time_into = mp.get_property("time-pos","")
   local sofa_file = retrieve_sofa_file_path();
   local command = retrieve_DSVSCA_exe_path() .. " -v " .. file_path .. " -s " .. sofa_file  
   -- local command =retrieve_DSVSCA_exe_path() .." -v " .. path .. " -s " .. sofa_file .. " &"
   -- Asynch version above, add a call to sleep(3 or so) to give it time to make header.
   os.execute(command)
   local filename_to_open = file_path:gsub("%.([%w%-]+)$", "") .. "-virtualized".. ".mp4"

   local curr_time_into = mp.get_property("time-pos","")
   mp.commandv("loadfile", filename_to_open, "replace", "start=+"..curr_time_into) -- Remove curr_time_into if doing asynch
   mp.set_property_native("pause", false) 
end

default_sofa_file_path = "~/Videos/SurroundVideos/subject_119.sofa";
default_DSVSCA_executable_path = "DSVSCA";
function retrieve_sofa_file_path()
   path = mp.get_opt("3d-sound-sofa-file",default_sofa_file_path);
   print("sofa " ..path);
   return path;
end

function retrieve_DSVSCA_exe_path()
   path = mp.get_opt("DSVSCA-executable-path",default_DSVSCA_executable_path);
   print("binary "..path);
   return path;
end


function sleep(n)
  os.execute("sleep " .. tonumber(n))
end

retrieve_sofa_file_path();
mp.add_key_binding("alt+w", "3D_AUDIOIFY", start_conversion_and_load_3d_audio)

