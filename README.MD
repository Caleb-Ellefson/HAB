Programming CPU2

```
openocd -f interface/stlink.cfg -f target/stm32wlx.cfg \
  -c "init" \
  -c "reset halt" \
  -c "stm32wlx unlock 0" \
  -c "mww 0x58004020 0x18" \
  -c "program /home/cellefson/zephyrproject/HAB/core2_m0/build/zephyr/zephyr.bin 0x0800C000 verify" \
  -c "reset run" \
  -c "shutdown"
```