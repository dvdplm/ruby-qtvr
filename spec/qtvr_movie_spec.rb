require File.dirname(__FILE__) + '/spec_helper.rb'

describe "QTVR::Movie" do
  describe "as an instance" do
    before do
      @movie = QTVR::Movie.new('spec/fixtures/good_qtvr.mov')
    end
    
    it "is initialized with a path to a QTVR movie" do
      lambda{
        movie = QTVR::Movie.new('spec/fixtures/good_qtvr.mov')
      }.should_not raise_error
    end

    it "raises if the path is invalid" do
      lambda{
        movie = QTVR::Movie.new('cannot/find/a/movie/here.mov')
      }.should raise_error(RuntimeError)
    end
    
    it "is a QTVR::Movie instance" do
      @movie.should be_an_instance_of(QTVR::Movie)
    end
    
    it "has a getter for the filename" do
      @movie.filename.should == 'spec/fixtures/good_qtvr.mov'
    end
    
    it "has a setter for the filename" do
      lambda{
        @movie.filename = "another/movie/file.mov"
      }.should_not raise_error
      @movie.filename.should == "another/movie/file.mov"
    end
    
    it "re-initializes the Movie instance when the filename changes" do
      pending # Not implemented
    end
    
    it "knows if the movie is active or not" do
      @movie.active?.should be_false # Not displaying the movie; not "playing" it at all
     
    end
  end
end