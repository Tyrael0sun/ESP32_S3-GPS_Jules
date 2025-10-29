Import("env")
import os

def after_build(source, target, env):
    print("Running post-build script to create factory binary...")

    # Get the project and build directories from the environment
    project_dir = env.get("PROJECT_DIR")
    build_dir = env.get("BUILD_DIR")

    # Construct the full command as a single string
    # We will use the esptool.py that is part of the PlatformIO toolchain
    python_exe = env.get("PYTHONEXE")

    # Get the path to the esptool.py from the PlatformIO tool package
    tool_esptoolpy_dir = env.PioPlatform().get_package_dir("tool-esptoolpy")
    esptool_py = os.path.join(tool_esptoolpy_dir, "esptool.py")

    cmd = (
        f'"{python_exe}" "{esptool_py}" --chip esp32s3 merge_bin '
        f'-o "{os.path.join(build_dir, "factory.bin")}" '
        f'--flash_mode dio --flash_size 4MB '
        f'0x0 "{os.path.join(build_dir, "bootloader.bin")}" '
        f'0x8000 "{os.path.join(build_dir, "partitions.bin")}" '
        f'0x10000 "{os.path.join(build_dir, "firmware.bin")}"'
    )

    # Execute the command
    env.Execute(cmd)
    print("Factory binary created at: " + os.path.join(build_dir, "factory.bin"))

# Register the post-build action
env.AddPostAction("$BUILD_DIR/firmware.bin", after_build)
