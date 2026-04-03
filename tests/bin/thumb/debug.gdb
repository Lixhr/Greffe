set architecture arm
set arm force-mode thumb         # remove if pure ARM, not Thumb
set exception-verbose off
set kernel-vmmap-via-page-tables off

file build/__greffe_workdir/test.bin.greffe
target remote :1234
