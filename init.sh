west init .
west update
pip install -r ./zephyr/scripts/requirements.txt
west blobs fetch hal_infineon
