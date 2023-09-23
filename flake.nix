{
  description = "fak";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-23.05";
    nickel.url = "github:tweag/nickel/1.2.1";
  };

  outputs = { self, nixpkgs, ... }@inputs:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      nickel = inputs.nickel.packages.${system}.default;
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = 
          let
            my-python-packages = ps: with ps; [
              (
                buildPythonPackage rec {
                  pname = "ch55xtool";
                  version = "1.0.4";
                  src = fetchPypi {
                    inherit pname version;
                    sha256 = "sha256-9EVzVHXVXlS7ohgC+wpvEK5DjQLvnRSf63BdrC0ug8w=";
                  };
                  propagatedBuildInputs = [
                    pkgs.python311Packages.pyusb
                  ];
                }
              )
            ];
          in
          with pkgs; [
            sdcc
            nickel
            meson
            ninja
            jq
            (python311.withPackages my-python-packages)
          ];
      };
    };
}
