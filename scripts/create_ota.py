# selforganized_802.15.4_network_with_esp32
# Copyright (c) 2024 Vedat Botuk.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

import argparse
import functools
import sys
import zlib
import zigpy.ota


def compress_data(data):
    """
    Compress data using zlib with the best compression level.
    """
    zobj = zlib.compressobj(level=zlib.Z_BEST_COMPRESSION)
    zdata = zobj.compress(data)
    zdata += zobj.flush()
    return zdata


def create_ota_image(filename, manufacturer_id, image_type, file_version, header_string):
    """
    Create a Zigbee OTA image with zlib-compressed data.
    """
    # Read the firmware data
    with open(filename, "rb") as f:
        data = f.read()

    # Compress the firmware data
    zdata = compress_data(data)

    # Build the OTA image header
    header = zigpy.ota.image.OTAImageHeader(
        upgrade_file_id=zigpy.ota.image.OTAImageHeader.MAGIC_VALUE,
        header_version=0x100,
        header_length=0,  # Will be updated later
        field_control=zigpy.ota.image.FieldControl(0),
        manufacturer_id=manufacturer_id,
        image_type=image_type,
        file_version=file_version,
        stack_version=2,
        header_string=header_string[:32],  # Truncate to 32 characters
        image_size=0,  # Will be updated later
    )

    # Build the OTA image with subelements
    image = zigpy.ota.image.OTAImage(
        header=header,
        subelements=[
            zigpy.ota.image.SubElement(
                tag_id=zigpy.ota.image.ElementTagId.UPGRADE_IMAGE,
                data=zdata,
            )
        ],
    )

    # Update header length and image size
    header.header_length = len(header.serialize())
    header.image_size = header.header_length + len(image.subelements.serialize())

    # Serialize the complete OTA image
    return image.serialize()


def parse_arguments():
    """
    Parse command-line arguments for the script.
    """
    any_int = functools.wraps(int)(functools.partial(int, base=0))
    parser = argparse.ArgumentParser(description="Create zlib-compressed Zigbee OTA file")
    parser.add_argument("filename", metavar="IMAGE", type=str, help="Firmware image filename")
    parser.add_argument("-m", "--manufacturer_id", metavar="MANUFACTUER_ID", type=any_int, required=True, help="Manufacturer ID")
    parser.add_argument("-i", "--image_type", metavar="IMAGE_ID", type=any_int, required=True, help="Image ID")
    parser.add_argument("-v", "--file_version", metavar="VERSION", type=any_int, required=True, help="File version")
    parser.add_argument("-s", "--header_string", metavar="HEADER_STRING", type=str, default="", help="Header String")
    return parser.parse_args()


def main():
    """
    Main entry point of the script.
    """
    args = parse_arguments()
    ota_image = create_ota_image(
        filename=args.filename,
        manufacturer_id=args.manufacturer_id,
        image_type=args.image_type,
        file_version=args.file_version,
        header_string=args.header_string,
    )
    sys.stdout.buffer.write(ota_image)


if __name__ == "__main__":
    main()
