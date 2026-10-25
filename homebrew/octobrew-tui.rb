class OctobrewTui < Formula
  desc "Terminal UI for Homebrew with the tabs, keys and themes of OctoPkg"
  homepage "https://github.com/cleberalves40/octobrew-tui"
  url "https://github.com/cleberalves40/octobrew-tui/archive/refs/tags/v0.4.0.tar.gz"
  sha256 "44673014a49c192781c6e74247fbc42f582833a2130afed5eb1643ccb4f5ad1d"
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
