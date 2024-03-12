{
  description = "fak";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-23.11";
    naersk.url = "github:nix-community/naersk";
    parts.url = "github:hercules-ci/flake-parts";
    devshell.url = "github:numtide/devshell";
  };

  outputs = inputs:
    inputs.parts.lib.mkFlake {inherit inputs;} {
      imports = [inputs.devshell.flakeModule];
      systems = ["x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin"];
      perSystem = { config, pkgs, ... }: let 
        naersk = pkgs.callPackage inputs.naersk {};
        wchisp = naersk.buildPackage rec {
          pname = "wchisp";
          version = "0.3-git";
          src = pkgs.fetchFromGitHub {
            owner = "ch32-rs";
            repo = pname;
            rev = "4b4787243ef9bc87cbbb0d95c7482b4f7c9838f1";
            hash = "sha256-Ju2DBv3R4O48o8Fk/AFXOBIsvGMK9hJ8Ogxk47f7gcU=";
          };
        };
        contents = with pkgs; [
          gcc
          sdcc
          nickel
          nls
          topiary
          meson
          python311
          ninja
        ];
      in {
        packages.container = pkgs.dockerTools.buildImage {
          name = "fak-devenv";
          tag = "latest";
          inherit contents;
        };
        devshells.default = { devshell = { packages = contents; }; };
        devshells.full = { devshell = { packages = contents ++ [wchisp]; }; };
      };
    };
}
