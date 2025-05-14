#!/bin/bash

show_header() {
    echo -e "\e[36m===============================\e[0m"
    echo -e "\e[36m         ESP-IDF 开发工具         \e[0m"
    echo -e "\e[36m===============================\e[0m"
}

show_menu() {
    show_header
    echo -e "\e[33m请选择要执行的命令：\e[0m"
    echo "1. 创建项目 (create-project)"
    echo "2. 设置目标芯片 (set-target)"
    echo "3. 配置项目 (menuconfig)"
    echo "4. 编译项目 (build)"
    echo "5. 打开容器终端 (bash)"
    echo "6. 启动串口服务器 (esp_rfc2217_server)"
    echo "7. 烧录程序 (flash)"
    echo "8. 串口监视器 (monitor)"
    echo -e "\e[31m0. 退出\e[0m"
}

show_help() {
    echo -e "\n\e[33m==============================="
    echo "      ESP-IDF 脚本使用帮助      "
    echo -e "===============================\e[0m"
    echo -e "\n1. 创建项目：create-project 创建基础项目结构。"
    echo -e "2. 设置目标芯片：set-target 指定芯片类型。"
    echo -e "3. 配置项目：menuconfig 进入配置界面。"
    echo -e "4. 编译项目：build 编译项目生成固件。"
    echo -e "5. 打开终端：bash 进入容器交互终端。"
    echo -e "6. 启动串口服务器：esp_rfc2217_server 提供网络串口。"
    echo -e "7. 烧录程序：flash 烧录固件至设备。"
    echo -e "8. 串口监视器：monitor 实时查看输出。"
    echo -e "0. 退出脚本。\n"
    echo -e "\e[33m===============================\e[0m"
}

select_project() {
    local root="$(dirname "$0")/project"
    [[ ! -d "$root" ]] && echo -e "\e[31m未找到 project 文件夹：$root\e[0m" && return 1

    local dirs=("$root"/*)
    local valid_dirs=()

    for d in "${dirs[@]}"; do
        [[ -f "$d/CMakeLists.txt" ]] && valid_dirs+=("$d")
    done

    [[ ${#valid_dirs[@]} -eq 0 ]] && echo -e "\e[31m未找到包含 CMakeLists.txt 的项目\e[0m" && return 1

    echo -e "\n\e[36m可用项目列表：\e[0m"
    for i in "${!valid_dirs[@]}"; do
        echo -e "\e[32m$((i+1)). $(basename "${valid_dirs[$i]}")\e[0m"
    done

    read -p "请选择项目 (输入编号): " index
    if [[ "$index" =~ ^[0-9]+$ ]] && (( index >= 1 && index <= ${#valid_dirs[@]} )); then
        echo "/project/$(basename "${valid_dirs[$((index-1))]}")"
    else
        echo -e "\e[31m输入无效。\e[0m"
        return 1
    fi
}

function ensure_esptool() {
    if ! command -v esp_rfc2217_server.py >/dev/null 2>&1; then
        echo "esp_rfc2217_server.py 未安装，尝试使用 pip install esptool"
        pip install --user esptool || {
            echo "安装失败，请手动安装 esptool。" >&2
            exit 1
        }
    fi
}

# ensure_esptool() {
#     local toolPath="scripts/esptool-linux-amd64"
#     if [[ ! -d "$toolPath" ]]; then
#         echo -e "\e[33m未找到 $toolPath，正在安装...\e[0m"
#         local installScript="scripts/install_esptool.py"
#         if [[ -f "$installScript" ]]; then
#             python "$installScript"
#             [[ ! -d "$toolPath" ]] && echo -e "\e[31m安装失败。\e[0m" && return 1
#         else
#             echo -e "\e[31m安装脚本不存在：$installScript\e[0m"
#             return 1
#         fi
#     fi
#     return 0
# }

run_command() {
    case $1 in
        1)
            read -p "请输入目标项目名称: " proj
            docker-compose run --rm esp-idf idf.py create-project "$proj"
            ;;
        2)
            projPath=$(select_project) || return
            read -p "请输入目标芯片名称（如 esp32s3）: " target
            docker-compose run --rm -w "$projPath" esp-idf idf.py set-target "$target"
            ;;
        3)
            projPath=$(select_project) || return
            docker-compose run --rm -w "$projPath" esp-idf idf.py menuconfig
            ;;
        4)
            projPath=$(select_project) || return
            docker-compose run --rm -w "$projPath" esp-idf idf.py build
            ;;
        5)
            docker-compose run --rm esp-idf bash
            ;;
        6)
            read -p "请输入目标串口设备（如 /dev/ttyUSB0）: " port
            ensure_esptool && (cd scripts/esptool-win64 && ./esp_rfc2217_server -v -p 4000 "$port")
            ;;
        7)
            projPath=$(select_project) || return
            docker-compose run --rm -w "$projPath" esp-idf idf.py --port "rfc2217://host.docker.internal:4000?ign_set_control" flash
            ;;
        8)
            projPath=$(select_project) || return
            docker-compose run --rm -w "$projPath" esp-idf idf.py --port "rfc2217://host.docker.internal:4000?ign_set_control" monitor
            ;;
        9)
            show_help
            ;;
        0)
            echo -e "\e[31m退出。\e[0m"
            exit 0
            ;;
        *)
            echo -e "\e[31m无效选择，请重试。\e[0m"
            ;;
    esac
}

while true; do
    show_menu
    read -p "请输入数字选择（如 1-8 或 0 退出）: " choice
    [[ "$choice" =~ ^[0-9]+$ ]] && run_command "$choice" || echo -e "\e[31m无效输入，请输入有效数字（0-9）。\e[0m"
done
