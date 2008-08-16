require File.dirname(__FILE__) + '/spec_helper.rb'

describe "QTVR module" do
  
  describe "QuickTime version" do
    it "knows the QT version" do
      QTVR.qt_version.should be_an_instance_of(Fixnum)
    end

    it "needs a QT version bigger than 7" do
      QTVR.qt_version.should > 7
    end
  end
end