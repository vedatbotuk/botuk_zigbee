#!/usr/bin/env python
#
# ESP Delta OTA Patch Generator Tool. This tool helps in generating the compressed patch file
# using BSDiff and Heatshrink algorithms
#
# SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0

import argparse
import os
import re
import tempfile
import hashlib
import sys
import esptool

try:
    import detools
except ImportError:
    print("Please install 'detools'. Use command `pip install -r tools/requirements.txt`")
    sys.exit(1)

# Magic Byte is created using command: echo -n "esp_delta_ota" | sha256sum
esp_delta_ota_magic = 0xfccdde10

MAGIC_SIZE = 4 # This is the size of the magic byte
DIGEST_SIZE = 32 # This is the SHA256 of the base binary    
HEADER_SIZE = 64
RESERVED_HEADER = HEADER_SIZE - (MAGIC_SIZE + DIGEST_SIZE) # This is the reserved header size

def calculate_sha256(file_path: str) -> str:
    """Calculate the SHA-256 hash of a file."""
    sha256_hash = hashlib.sha256()
    
    with open(file_path, "rb") as f:
        # Read the file in chunks to avoid memory issues for large files
        for byte_block in iter(lambda: f.read(4096), b""):
            sha256_hash.update(byte_block)
    
    # Return the hex representation of the hash
    return sha256_hash.hexdigest()

def create_patch(chip: str, base_binary: str, new_binary: str, patch_file_name: str) -> None:
    command = ['--chip', chip, 'image_info', base_binary]
    output = sys.stdout
    sys.stdout = tempfile.TemporaryFile(mode='w+')
    try:
        esptool.main(command)
        sys.stdout.seek(0)
        content = sys.stdout.read()
    except Exception as e:
        print(f"Error during esptool command execution: {e}")
    finally:
        sys.stdout.close()
        sys.stdout = output

    x = re.search(r"Validation Hash: ([A-Za-z0-9]+) \(valid\)", content, re.IGNORECASE)
    
    if x is None:
        print("Failed to find validation hash in base binary.")
        return
        
    # --- FIX: Create a unique temp file name using the base and new binary names ---
    # This prevents the race condition when multiple CMake targets run at once
    unique_id = os.path.basename(patch_file_name).replace(".patch", "")
    patch_file_without_header = f"patch_file_temp_{unique_id}.bin"
    # -------------------------------------------------------------------------------

    try:
        with open(base_binary, 'rb') as b_binary, open(new_binary, 'rb') as n_binary, open(patch_file_without_header, 'wb') as p_binary:
            detools.create_patch(b_binary, n_binary, p_binary, compression='heatshrink') # b_binary is the base binary, n_binary is the new binary, p_binary is the patch file without header

        with open(patch_file_without_header, "rb") as p_binary, open(patch_file_name, "wb") as patch_file:
            patch_file.write(esp_delta_ota_magic.to_bytes(MAGIC_SIZE, 'little'))
            patch_file.write(bytes.fromhex(x[1]))
            patch_file.write(bytearray(RESERVED_HEADER))
            patch_file.write(p_binary.read())    
    except Exception as e:
        print(f"Error during patch creation: {e}")
    finally:
        if os.path.exists(patch_file_without_header):
            os.remove(patch_file_without_header)

    print("Patch created successfully.")
    # Verifying the created patch file
    verify_patch(base_binary, patch_file_name, new_binary)
def verify_patch(base_binary: str, patch_to_verify: str, new_binary: str) -> None:
    # 1. Read patch content
    with open(patch_to_verify, "rb") as original_file:
        original_file.seek(HEADER_SIZE)
        patch_content = original_file.read()

    # We use None to track if files were created for cleanup
    temp_patch_name = None
    temp_output_name = None

    try:
        # 2. Create a unique temporary file for the patch source
        with tempfile.NamedTemporaryFile(delete=False) as temp_patch:
            temp_patch.write(patch_content)
            temp_patch.flush()
            temp_patch_name = temp_patch.name

        # 3. Create a unique temporary filename for the reconstructed output
        # We use delete=False because detools needs to write to the path
        with tempfile.NamedTemporaryFile(delete=False) as temp_output:
            temp_output_name = temp_output.name

        # 4. Apply patch using unique names
        detools.apply_patch_filenames(base_binary, temp_patch_name, temp_output_name)

        # 5. Calculate SHAs using the unique output name
        sha_of_new_created_binary = calculate_sha256(temp_output_name)
        sha_of_new_binary = calculate_sha256(new_binary)
        
        if sha_of_new_created_binary == sha_of_new_binary:
            print("Patch file verified successfully")
        else:
            # We raise an error so the build fails if verification fails
            raise ValueError("Failed to verify the patch: SHA256 mismatch")

    except Exception as e:
        print(f"Error during patch verification: {e}")
        # Re-raise to ensure the build stops on failure
        raise 
    finally:
        # 6. Cleanup both unique files
        if temp_patch_name and os.path.exists(temp_patch_name):
            os.remove(temp_patch_name)
        if temp_output_name and os.path.exists(temp_output_name):
            os.remove(temp_output_name)

def main() -> None:
    if len(sys.argv) < 2:
        print("Usage: python esp_delta_ota_patch_gen.py create_patch/verify_patch [arguments]")
        sys.exit(1)

    command = sys.argv[1]
    parser = argparse.ArgumentParser('Delta OTA Patch Generator Tool')

    if command == 'create_patch':
        parser.add_argument('--chip', help="Target", default="esp32")
        parser.add_argument('--base_binary', help="Path of Base Binary for creating the patch", required=True)
        parser.add_argument('--new_binary', help="Path of New Binary for which patch has to be created", required=True)
        parser.add_argument('--patch_file_name', help="Patch file path", default="patch.bin")
        args = parser.parse_args(sys.argv[2:])
        create_patch(args.chip, args.base_binary, args.new_binary, args.patch_file_name)
    elif command == 'verify_patch':
        parser.add_argument('--base_binary', help="Path of Base Binary for verifying the patch", required=True)
        parser.add_argument('--patch_file_name', help="Patch file path", required=True)
        parser.add_argument('--new_binary', help="Path of New Binary for verifying the patch", required=True)
        args = parser.parse_args(sys.argv[2:])
        verify_patch(args.base_binary, args.patch_file_name, args.new_binary)
    else:
        print("Invalid command. Use 'create_patch' or 'verify_patch'.")
        sys.exit(1)

if __name__ == '__main__':
    main()
