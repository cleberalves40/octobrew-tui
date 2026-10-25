class OctobrewTui < Formula
  desc "Terminal UI for Homebrew with the tabs, keys and themes of OctoPkg"
  homepage "https://github.com/cleberalves40/octobrew-tui"
  url "https://github.com/cleberalves40/octobrew-tui/archive/refs/tags/v1.0.1.tar.gz"
  sha256 "549d7ee095571be91dfa71ada356e473747c620a16aee1c768acde4bd19267a8"
  license "GPL-2.0-or-later"
  head "https://github.com/cleberalves40/octobrew-tui.git", branch: "main"

  depends_on "cmake" => :build
  depends_on "ninja" => :build
  depends_on "ftxui"

  def install
    system "cmake", "-S", ".", "-B", "build", "-G", "Ninja", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    assert_match "octobrew-tui #{version}", shell_output("#{bin}/octobrew-tui --version")
    assert_match "Homebrew", shell_output("#{bin}/octobrew-tui --help")
    # Uma opção desconhecida sai com 2, sem abrir a tela.
    assert_match "--remote", shell_output("#{bin}/octobrew-tui --nao-existe 2>&1", 2)
  end
end
