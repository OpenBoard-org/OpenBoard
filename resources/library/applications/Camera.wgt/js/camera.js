// size options
const sizeOptions = [
  { 'label': "S", 'size': 320 },
  { 'label': "M", 'size': 640 },
  { 'label': "L", 'size': 960 },
  { 'label': "XL", 'size': 1280 }
];

async function init() {
  if (!"mediaDevices" in navigator ||
      !"getUserMedia" in navigator.mediaDevices) {
    handleError("Camera API is not available in your browser");
    return;
  }

  // get page elements
  const video         = document.getElementById("video");
  const buttons       = document.getElementById("buttons");
  const buttonsLeft   = document.getElementById("buttons-left");
  const snapshot      = document.getElementById("snapshot");
  const rotleft       = document.getElementById("rotleft");
  const rotright      = document.getElementById("rotright");
  const canvas        = document.getElementById("canvas");
  const nocamera      = document.getElementById("nocamera");
  const selectCamera  = document.getElementById("selectCamera");
  const videoSelect   = document.getElementById("videoSelect");
  const sizeSelect    = document.getElementById("sizeSelect");
  const sizeIcon      = document.getElementById("sizeIcon");
  const play          = document.getElementById("play");
  const pause         = document.getElementById("pause");
  const glass         = document.getElementById("glass");

  // current camera;
  let camera;

  // current camera label
  let cameraLabel;

  // camera status
  let playing = true;

  // current video stream
  let videoStream;

  // orientation
  let orientation = parseInt(await sankore.async.preference("orientation", "0"));

  // size of video in widget
  let videoSize;

  // snapshot size
  let size;

  // cropper
  let cropper;

  let cropOptions = {
    viewMode: 0,
    guides: false,
    center: false,
    background: false,
    rotatable: false,
    scalable: false,
    zoomable: false,
    autoCropArea: 0.99,
    toggleDragModeOnDblclick: false,
    ready() {
      resizeCropper(videoSize);
      showCropper(true);
    },
    cropend() {
      updateSizes();
    }
  };

  // translate tooltips
  translate();

  // setup size options
  for (const sizeOption of sizeOptions) {
    const input = document.createElement('input');
    input.setAttribute("type", "radio");
    input.setAttribute("name", "size");
    input.setAttribute("id", sizeOption.label);
    input.value = sizeOption.size;

    const label = document.createElement('label');
    label.appendChild(input);
    const name = document.createTextNode(sizeOption.label);
    label.appendChild(name);

    const div = document.createElement('div');
    div.onclick = () => {
      setSize(sizeOption.label);
    };
    div.appendChild(label);

    sizeSelect.appendChild(div);
  }

  let sizeId = await sankore.async.preference("size", "M")
  setSize(sizeId);

  // connect hover slots
  window.widget.onenter.connect(() => {
    buttons.style.display = "flex";
    buttonsLeft.style.display = "flex";
    if (videoStream) {
      (playing ? pause : play).style.display = "block";
    }

    showCropper(true);
  });

  window.widget.onleave.connect(() => {
    buttons.style.display = "none";
    buttonsLeft.style.display = "none";
    pause.style.display = "none";
    play.style.display = "none";
    showCropper(false);
  });

  // connect event handler
  // rescan camera list
  selectCamera.onmouseenter = () => {
    getDevices().then(gotDevices);
  }

  // resize widget to video aspect ratio
  video.onresize = () => {
    if (!cropper) {
      cropper = new Cropper(glass, cropOptions);
    }

    let videoSize = layoutVideo();
    sankore.resize(videoSize.width, videoSize.height);
  };

  // adjust video size when widget is resized
  window.onresize = layoutVideo;

  // rotate
  rotleft.onclick = () => {
    orientation = (orientation + 3) % 4;
    sankore.setPreference("orientation", orientation);
    let videoSize = layoutVideo();
    sankore.resize(videoSize.width, videoSize.height);
  };

  rotright.onclick = () => {
    orientation = (orientation + 1) % 4;
    sankore.setPreference("orientation", orientation);
    let videoSize = layoutVideo();
    sankore.resize(videoSize.width, videoSize.height);
  };

  // play/pause
  play.onclick = () => {
    video.play();
    playing = true;
    play.style.display = "none";
    pause.style.display = "block";
  };

  pause.onclick = () => {
    video.pause();
    playing = false;
    pause.style.display = "none";
    play.style.display = "block";
  };

  // take snapshot
  snapshot.onclick = (event) => {
    event.cancelBubble = true;

    let clipBox = getClipBox();

    // compute cropping factors
    let cleft = clipBox.left / videoSize.width;
    let cwidth = clipBox.width / videoSize.width;
    let ctop = clipBox.top / videoSize.height;
    let cheight = clipBox.height / videoSize.height;

    let vleft;
    let vwidth;
    let vtop;
    let vheight;

    // cropped video coordinates depending on orientation
    switch (orientation) {
      case 0:
        vleft = cleft * video.videoWidth;
        vwidth = cwidth * video.videoWidth;
        vtop = ctop * video.videoHeight;
        vheight = cheight * video.videoHeight;
        break;
      case 1:
        vleft = ctop * video.videoWidth;
        vwidth = cheight * video.videoWidth;
        vtop = (1 - cleft - cwidth) * video.videoHeight;
        vheight = cwidth * video.videoHeight;
        break;
      case 2:
        vleft = (1 - cleft - cwidth) * video.videoWidth;
        vwidth = cwidth * video.videoWidth;
        vtop = (1 - ctop - cheight) * video.videoHeight;
        vheight = cheight * video.videoHeight;
        break;
      case 3:
        vleft = (1 - ctop - cheight) * video.videoWidth;
        vwidth = cheight * video.videoWidth;
        vtop = cleft * video.videoHeight;
        vheight = cwidth * video.videoHeight;
        break;
    }

    const aspectRatio = clipBox.width / clipBox.height;

    if (aspectRatio < 1) {
      canvas.height = size.value;
      canvas.width = canvas.height * aspectRatio;
    } else {
      canvas.width = size.value;
      canvas.height = canvas.width / aspectRatio;
    }

    let ctx = canvas.getContext("2d");
    ctx.setTransform(1, 0, 0, 1, 0, 0);
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.translate(canvas.width / 2, canvas.height / 2);
    ctx.rotate(orientation * Math.PI / 2);

    if (orientation % 2) {
      ctx.translate(-canvas.height / 2, -canvas.width / 2);
      ctx.drawImage(video, vleft, vtop, vwidth, vheight, 0, 0, canvas.height, canvas.width);
    } else {
      ctx.translate(-canvas.width / 2, -canvas.height / 2);
      ctx.drawImage(video, vleft, vtop, vwidth, vheight, 0, 0, canvas.width, canvas.height);
    }

    let data = canvas.toDataURL("image/jpeg");
    sankore.addObject(data, 0, 0, 0, 0, false);
  };

  // layout video
  function layoutVideo() {
    if (video.videoHeight === 0 || video.videoWidth === 0) {
      return;
    }

    let longWindow = Math.max(window.innerHeight, window.innerWidth);
    let width;
    let height;

    video.style.transform = "rotate(" + 90 * orientation + "deg)";

    if (orientation % 2 === 0) {
      // for landscape:
      // use longest side for width
      // adapt the height according to aspect ratio
      width = longWindow;
      height = width * video.videoHeight / video.videoWidth;

      video.style.width = width + "px";
      video.style.height = height + "px";
      video.style.top = "0px";
      video.style.left = "0px";
    } else {
      // for portrait:
      // use longest side for height
      // adapt width according to aspect ratio
      height = longWindow;
      width = height * video.videoHeight / video.videoWidth;

      video.style.width = height + "px";
      video.style.height = width + "px";
      let offset = (height - width) / 2;
      video.style.top = offset + "px";
      video.style.left = -offset + "px";
    }

    glass.style.width = width + "px";
    glass.style.height = height + "px";

    if (videoSize === undefined || Math.abs(width - videoSize.width) > 2 || Math.abs(height - videoSize.height) > 2) {
      videoSize = {
        'width': width,
        'height': height
      };

      // delay until cropper is ready
      setTimeout(() => {
        resizeCropper(videoSize);
        updateSizes();
      }, 100);
    }

    return videoSize;
  };

  // stop video stream
  function stopVideoStream() {
    if (videoStream) {
      videoStream.getTracks().forEach((track) => {
        track.stop();
      });
    }
  }

  // request list of media devices
  function getDevices() {
    return navigator.mediaDevices.enumerateDevices();
  }

  // extract video devices and build selection options
  function gotDevices(deviceInfos) {
    // delete list
    while (videoSelect.hasChildNodes()) {
      videoSelect.removeChild(videoSelect.firstChild);
    }
    // populate list
    for (const deviceInfo of deviceInfos) {
      if (deviceInfo.kind === 'videoinput') {
        const input = document.createElement('input');
        input.setAttribute("type", "radio");
        input.setAttribute("name", "source");
        input.setAttribute("id", deviceInfo.deviceId);
        input.value = deviceInfo.deviceId;

        const label = document.createElement('label');
        label.appendChild(input);
        const text = deviceInfo.label || `Camera ${videoSelect.length + 1}`;
        const name = document.createTextNode(text);
        label.appendChild(name);

        const div = document.createElement('div');
        div.onclick = () => {
          setCamera(deviceInfo.deviceId);
        };
        div.appendChild(label);

        videoSelect.appendChild(div);
      }
    }

    // select previously selected video source
    let selected = undefined;

    if (cameraLabel) {
      selected = [...videoSelect.children].find(entry => entry.firstChild.textContent === cameraLabel);
    }

    if (selected) {
      selected.firstChild.firstChild.checked = true;
    } else {
      // camera no longer available
      handleError();
    }
  }

  // request stream for selected device
  function getStream() {
    stopVideoStream();
    const videoSource = camera;
    const constraints = {
      video: {
        deviceId: videoSource ? {exact: videoSource} : undefined,
        width: { ideal: 1920 },
        height: { ideal: 1080 }
      }
    };
    return navigator.mediaDevices.getUserMedia(constraints).
      then(gotStream).catch(handleError);
  }

  // assign stream to video element
  function gotStream(stream) {
    videoStream = stream;
    cameraLabel = stream.getVideoTracks()[0].label;
    let selected = [...videoSelect.children].find(entry => entry.firstChild.textContent === cameraLabel);
    if (selected) {
      selected.firstChild.firstChild.checked = true;
    }

    video.srcObject = stream;
    video.style.display = "unset";
    nocamera.style.display = "none";
  }

  // display "no camera" symbol
  function handleError(error) {
    videoStream = undefined;
    camera = undefined;
    cameraLabel = undefined;
    video.style.display = "none";
    pause.style.display = "none";
    play.style.display = "none";
    nocamera.style.display = "flex";
    showCropper(false);
  }

  // change size
  function setSize(id) {
    size = document.getElementById(id);
    size.checked = true;
    sizeIcon.textContent = id;
    sankore.setPreference("size", id);
  }

  // select camera
  function setCamera(id) {
    const input = document.getElementById(id);

    if (input) {
      input.checked = true;
      camera = input.id;
      getStream();
    }
  }

  // show or hide cropper
  function showCropper(show) {
    const cropperContainer = document.querySelector(".cropper-container");

    if (cropperContainer) {
      if (show && videoStream) {
        cropperContainer.style.display = "unset";
      } else {
        cropperContainer.style.display = "none";
      }
    }
  }

  // resize cropper
  function resizeCropper(size) {
    if (cropper) {
      cropper.setCropBoxData({
        left: 2,
        top: 2,
        width: size.width - 4,
        height: size.height - 4
      });
    }
  }

  // get effective size of crop box
  function getClipBox() {
    let cropBox = cropper.getCropBoxData();
    let clipBox = { ...cropBox };

    // round if close to border
    if (cropBox.left < 4) clipBox.left = 0;
    if (cropBox.top < 4) clipBox.top = 0;
    if (videoSize.width - cropBox.left - cropBox.width < 4) clipBox.width = videoSize.width - clipBox.left;
    if (videoSize.height - cropBox.top - cropBox.height < 4) clipBox.height = videoSize.height - clipBox.top;

    return clipBox;
  }

  // update pixel sizes in size selection menu
  function updateSizes() {
    const clipBox = getClipBox();
    const longSide = Math.max(clipBox.width, clipBox.height);

    for (entry of sizeSelect.children) {
      const pixels = entry.firstChild.firstChild.value;
      const factor = pixels / longSide;
      const x = Math.round(clipBox.width * factor);
      const y = Math.round(clipBox.height * factor);
      const label = entry.firstChild.firstChild.id + " (" + x + "x" + y + ")";
      entry.firstChild.childNodes[1].textContent = label;
    }
  }

  // initialize
  getDevices().then(gotDevices).then(getStream);
}
