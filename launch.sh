BOARD=rpi_pico/rp2040/w

rm ./src/buff.h ./build/ext1.llext
west build -p -b $BOARD -t extension1
xxd -i ./build/ext1.llext > ./src/buff.h
sed -i "s/__build_ext1_llext/ext1_buf/g" ./src/buff.h
# En principio los argumentos de después de -- solo son necesarios para la pico
if [ "$BOARD" = "rpi_pico/rp2040/w" ]; then
    west build -b $BOARD -- -DRPI_PICO_DEBUG_ADAPTER=cmsis-dap -DOPENOCD=openocd/openocd -DOPENOCD_DEFAULT_PATH=openocd/scripts
else
    west build -b $BOARD
fi
