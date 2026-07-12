import os
import subprocess
import datetime
import sys
import re


def get_git_username():
    try:
        return subprocess.check_output(["git", "config", "user.name"]).strip().decode()
    except:
        return None


def get_windows_username():
    return os.getenv('USERNAME')

# Get USERNAME from environment variables file .env


def get_env_username():
    try:
        with open('.env', 'r') as file:
            content = file.read()
            pattern = re.compile(r"USERNAME=(.*)")
            match = pattern.search(content)
            if match:
                return match.group(1)
    except:
        return None


def update_last_edit(file_path):

    git_username = get_env_username()

    if not git_username:
        git_username = get_git_username()
    if not git_username:
        git_username = get_windows_username()

    if not git_username:
        print("User name not found. Exiting.")
        return

    timestamp = datetime.datetime.now().strftime("%Y/%m/%d %H:%M:%S")
    last_edit_format = "Last Edit: {} ({})\n"

    mission_last_edit_format = "#define MISSION_LAST_EDITOR \t\"{}\"\n"
    mission_last_edit_t_format = "#define MISSION_LAST_EDITED \t\"{}\"\n"

    pattern = re.compile(r"(\s*)Last Edit:.*\n?")
    pattern_m_le = re.compile(r"(\s*)#define MISSION_LAST_EDITOR.*\n?")
    pattern_m_le_t = re.compile(r"(\s*)#define MISSION_LAST_EDITED.*\n?")

    with open(file_path, 'r+') as file:
        content = file.read()
        file.seek(0)
        file.truncate()

        # Regex to match the last edit line with any leading whitespace

        match = pattern.search(content)
        if match:
            # Replace while preserving the leading whitespace
            last_edit_line = f"{match.group(1)}{last_edit_format.format(timestamp, git_username)}"
            content = pattern.sub(last_edit_line, content)

        match = pattern_m_le.search(content)
        if match:
            # Replace while preserving the leading whitespace
            last_edit_line = f"{match.group(1)}{mission_last_edit_format.format(git_username)}"
            content = pattern_m_le.sub(last_edit_line, content)

        match = pattern_m_le_t.search(content)
        if match:
            # Replace while preserving the leading whitespace
            last_edit_line = f"{match.group(1)}{mission_last_edit_t_format.format(timestamp)}"
            content = pattern_m_le_t.sub(last_edit_line, content)

        file.write(content)


# Check if a file path is provided
if len(sys.argv) > 1:
    file_path = sys.argv[1]
    update_last_edit(file_path)
else:
    print("No file path provided.")
