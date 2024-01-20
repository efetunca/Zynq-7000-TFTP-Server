# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\DEVELOPER\Projects\Embedded\Zynq-7000-TFTP-Server\TFTP_server-platform\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\DEVELOPER\Projects\Embedded\Zynq-7000-TFTP-Server\TFTP_server-platform\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {TFTP_server-platform}\
-hw {C:\DEVELOPER\Projects\Embedded\Zynq-7000-TFTP-Server\zc702-bitstream.xsa}\
-proc {ps7_cortexa9_0} -os {standalone} -fsbl-target {psu_cortexa53_0} -out {C:/DEVELOPER/Projects/Embedded/Zynq-7000-TFTP-Server}

platform write
platform generate -domains 
platform active {TFTP_server-platform}
domain active {zynq_fsbl}
bsp reload
bsp setlib -name lwip211 -ver 1.3
bsp config set_fs_rpath "2"
bsp config use_chmod "true"
bsp config use_lfn "2"
bsp write
bsp reload
catch {bsp regenerate}
domain active {standalone_domain}
bsp reload
bsp setlib -name lwip211 -ver 1.3
bsp setlib -name xilffs -ver 4.4
bsp config set_fs_rpath "2"
bsp config use_chmod "true"
bsp config use_lfn "2"
bsp write
bsp reload
catch {bsp regenerate}
platform generate
