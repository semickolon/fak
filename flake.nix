{
  description = "fak";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-23.11";
    naersk.url = "github:nix-community/naersk";
  };

  outputs = { self, nixpkgs, ... }@inputs:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
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
        sdcc
        nickel
        nls
        topiary
        meson
        python311
        ninja
      ];
    in
    {
      devShells.${system} = {
        default = pkgs.mkShell {
          packages = contents;
        };
        full = pkgs.mkShell {
          packages = contents ++ [ wchisp ];
        };
      };

      packages.${system}.container = pkgs.dockerTools.buildImage {
        name = "fak-devenv";
        tag = "latest";
        inherit contents;
      };
    };
}
