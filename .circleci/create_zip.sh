#!/bin/bash
set -e

VERSION=$1
ARTIFACTS_DIR="artifacts"
RESOURCES_DIR="artifacts/resources"
MANIFEST_DIR="artifacts/manifest"
COPYRIGHT_DIR="artifacts/copyright"
LIB_DIR="artifacts/lib"
INCLUDE_DIR="artifacts/include"
DEBUG_LIB_DIR="artifacts/debug/lib"

main() {
  
  if [[ ! -d "$ARTIFACTS_DIR" ]]; then
    mkdir -p "$ARTIFACTS_DIR"
  fi

  copy_resources
  copy_manifest
  copy_libs "Release" "$LIB_DIR"
  copy_libs "Debug" "$DEBUG_LIB_DIR"
  copy_includes "$INCLUDE_DIR"
  copy_copyright
  compact_files
}

copy_resources() {
  rm -f "$RESOURCES_DIR"/*
  if [[ ! -d "$RESOURCES_DIR" ]]; then
    mkdir -p "$RESOURCES_DIR"
  fi

  cp src/CertificateResources/* "$RESOURCES_DIR"
}

copy_manifest() {
  rm -f "$MANIFEST_DIR"/*
  if [[ ! -d "$MANIFEST_DIR" ]]; then
    mkdir -p "$MANIFEST_DIR"
  fi

  cp src/manifest/* "$MANIFEST_DIR"
}

copy_libs() {
  build_type="$1"
  lib_dir="$2"

  rm -f "${lib_dir}"/*
  if [[ ! -d "$lib_dir" ]]; then
    mkdir -p "$lib_dir"
  fi

  cp "src/x86/${build_type}/WindowsPackageManager/WindowsPackageManager.lib" "$lib_dir"
  cp "src/x86/${build_type}/WindowsPackageManager/WindowsPackageManager.dll" "$lib_dir"
}

copy_includes() {
  include_dir="$1"

  rm -f "${include_dir}"/*
  if [[ ! -d "$include_dir" ]]; then
    mkdir -p "$include_dir"
  fi

  cp "src/WindowsPackageManager/WindowsPackageManager.h" "$include_dir"
}

copy_copyright() {
  rm -f "$COPYRIGHT_DIR"/*
  if [[ ! -d "$COPYRIGHT_DIR" ]]; then
    mkdir -p "$COPYRIGHT_DIR"
  fi

  cp LICENSE "$COPYRIGHT_DIR"

}

compact_files() {
  package="winget-cli_${VERSION}.zip"
  rm -f "${package}"
  pwsh -c "Compress-Archive -Path artifacts/* -DestinationPath $package"
}

main "$@"
